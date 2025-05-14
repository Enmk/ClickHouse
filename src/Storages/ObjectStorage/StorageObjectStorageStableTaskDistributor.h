#pragma once

#include <Client/Connection.h>
#include <Common/Logger.h>
#include <Interpreters/Cluster.h>
#include <Storages/ObjectStorage/StorageObjectStorageSource.h>
#include <Storages/ObjectStorageQueue/ObjectStorageQueueSource.h>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <vector>
#include <mutex>
#include <memory>

namespace DB
{

class StorageObjectStorageStableTaskDistributor
{
public:
    StorageObjectStorageStableTaskDistributor(
        std::shared_ptr<IObjectIterator> iterator_,
        std::vector<std::string> ids_of_nodes_);

    std::optional<String> getNextTask(size_t number_of_current_replica);

    /// Insert objects back to unprocessed files
    void rescheduleTasksFromReplica(size_t number_of_current_replica);

private:
    size_t getReplicaForFile(const String & file_path);
    std::optional<String> getPreQueuedFile(size_t number_of_current_replica);
    std::optional<String> getMatchingFileFromIterator(size_t number_of_current_replica);
    std::optional<String> getAnyUnprocessedFile(size_t number_of_current_replica);

    std::shared_ptr<IObjectIterator> iterator;

    std::vector<std::vector<String>> connection_to_files;
    std::unordered_set<String> unprocessed_files;

    std::vector<std::string> ids_of_nodes;
    std::unordered_map<size_t, std::list<String>> replica_to_files_to_be_processed;

    std::mutex mutex;
    bool iterator_exhausted = false;

    LoggerPtr log = getLogger("StorageClusterTaskDistributor");
};

}
