#include "StorageObjectStorageStableTaskDistributor.h"
#include <Common/SipHash.h>
#include <consistent_hashing.h>
#include <optional>

namespace DB
{

namespace ErrorCodes
{
    extern const int LOGICAL_ERROR;
    extern const int CANNOT_READ_ALL_DATA;
};

StorageObjectStorageStableTaskDistributor::StorageObjectStorageStableTaskDistributor(
    std::shared_ptr<IObjectIterator> iterator_,
    std::vector<std::string> ids_of_nodes_)
    : iterator(std::move(iterator_))
    , connection_to_files(ids_of_nodes_.size())
    , ids_of_nodes(ids_of_nodes_)
    , iterator_exhausted(false)
{
    size_t nodes = ids_of_nodes.size();
    for (size_t i = 0; i < nodes; ++i)
        replica_to_files_to_be_processed[i] = std::list<String>{};
}

std::optional<String> StorageObjectStorageStableTaskDistributor::getNextTask(size_t number_of_current_replica)
{
    LOG_TRACE(
        log,
        "Received a new connection from replica {} looking for a file",
        number_of_current_replica
    );

    auto processed_file_list_ptr = replica_to_files_to_be_processed.find(number_of_current_replica);
    if (processed_file_list_ptr == replica_to_files_to_be_processed.end())
        throw Exception(
            ErrorCodes::LOGICAL_ERROR,
            "Replica number {} was marked as lost, can't set task for it anymore",
            number_of_current_replica
        );

    // 1. Check pre-queued files first
    std::optional<String> file = getPreQueuedFile(number_of_current_replica);
    // 2. Try to find a matching file from the iterator
    if (!file.has_value())
        file = getMatchingFileFromIterator(number_of_current_replica);
    // 3. Process unprocessed files if iterator is exhausted
    if (!file.has_value())
        file = getAnyUnprocessedFile(number_of_current_replica);

    if (file.has_value())
        processed_file_list_ptr->second.push_back(*file);

    return file;
}

size_t StorageObjectStorageStableTaskDistributor::getReplicaForFile(const String & file_path)
{
    size_t nodes_count = ids_of_nodes.size();

    /// Trivial case
    if (nodes_count < 2)
        return 0;

    /// Rendezvous hashing
    size_t best_id = 0;
    UInt64 best_weight = sipHash64(ids_of_nodes[0] + file_path);
    for (size_t id = 1; id < nodes_count; ++id)
    {
        UInt64 weight = sipHash64(ids_of_nodes[id] + file_path);
        if (weight > best_weight)
        {
            best_weight = weight;
            best_id = id;
        }
    }
    return best_id;
}

std::optional<String> StorageObjectStorageStableTaskDistributor::getPreQueuedFile(size_t number_of_current_replica)
{
    std::lock_guard lock(mutex);

    if (connection_to_files.size() <= number_of_current_replica)
        throw Exception(
            ErrorCodes::LOGICAL_ERROR,
            "Replica number {} is out of range. Expected range: [0, {})",
            number_of_current_replica,
            connection_to_files.size()
        );

    auto & files = connection_to_files[number_of_current_replica];

    while (!files.empty())
    {
        String next_file = files.back();
        files.pop_back();

        auto it = unprocessed_files.find(next_file);
        if (it == unprocessed_files.end())
            continue;

        unprocessed_files.erase(it);

        LOG_TRACE(
            log,
            "Assigning pre-queued file {} to replica {}",
            next_file,
            number_of_current_replica
        );

        return next_file;
    }

    return std::nullopt;
}

std::optional<String> StorageObjectStorageStableTaskDistributor::getMatchingFileFromIterator(size_t number_of_current_replica)
{
    {
        std::lock_guard lock(mutex);
        if (iterator_exhausted)
            return std::nullopt;
    }

    while (true)
    {
        ObjectInfoPtr object_info;

        {
            std::lock_guard lock(mutex);
            object_info = iterator->next(0);

            if (!object_info)
            {
                iterator_exhausted = true;
                break;
            }
        }

        String file_path;

        auto archive_object_info = std::dynamic_pointer_cast<StorageObjectStorageSource::ArchiveIterator::ObjectInfoInArchive>(object_info);
        if (archive_object_info)
        {
            file_path = archive_object_info->getPathToArchive();
        }
        else
        {
            file_path = object_info->getPath();
        }

        size_t file_replica_idx = getReplicaForFile(file_path);
        if (file_replica_idx == number_of_current_replica)
        {
            LOG_TRACE(
                log,
                "Found file {} for replica {}",
                file_path,
                number_of_current_replica
            );

            return file_path;
        }

        // Queue file for its assigned replica
        {
            std::lock_guard lock(mutex);
            unprocessed_files.insert(file_path);
            connection_to_files[file_replica_idx].push_back(file_path);
        }
    }

    return std::nullopt;
}

std::optional<String> StorageObjectStorageStableTaskDistributor::getAnyUnprocessedFile(size_t number_of_current_replica)
{
    std::lock_guard lock(mutex);

    if (!unprocessed_files.empty())
    {
        auto it = unprocessed_files.begin();
        String next_file = *it;
        unprocessed_files.erase(it);

        LOG_TRACE(
            log,
            "Iterator exhausted. Assigning unprocessed file {} to replica {}",
            next_file,
            number_of_current_replica
        );

        return next_file;
    }

    return std::nullopt;
}

void StorageObjectStorageStableTaskDistributor::rescheduleTasksFromReplica(size_t number_of_current_replica)
{
    LOG_INFO(log, "Replica {} is marked as lost, tasks are returned to queue", number_of_current_replica);
    std::lock_guard lock(mutex);

    auto processed_file_list_ptr = replica_to_files_to_be_processed.find(number_of_current_replica);
    if (processed_file_list_ptr == replica_to_files_to_be_processed.end())
        throw Exception(
            ErrorCodes::LOGICAL_ERROR,
            "Replica number {} was marked as lost already",
            number_of_current_replica
        );

    if (replica_to_files_to_be_processed.size() < 2)
        throw Exception(
            ErrorCodes::CANNOT_READ_ALL_DATA,
            "All replicas were marked as lost"
        );

    for (const auto & file_path : processed_file_list_ptr->second)
        unprocessed_files.insert(file_path);
    replica_to_files_to_be_processed.erase(number_of_current_replica);
}

}
