#pragma once

#include <fstream>
#include <filesystem>
#include <string>

#include "../third_party/json.h"
using json = nlohmann::json;

namespace buzzhouse {

class FuzzConfig {
public:
	uint32_t seed = 0, max_depth = 3, max_width = 3, max_databases = 4,
			 max_functions = 4, max_tables = 10, max_views = 5;
	bool read_log = false;
	std::filesystem::path log_path = std::filesystem::temp_directory_path() / "out.sql",
						  db_file_path = std::filesystem::temp_directory_path() / "db";

	FuzzConfig(const std::string &path) {
		std::ifstream ifs(path);
		const json jdata = json::parse(ifs);

		for (const auto& [key, value] : jdata.items()) {
			if (key == "db_file_path") {
				db_file_path = std::filesystem::path(value);
			} else if (key == "log_path") {
				log_path = std::filesystem::path(value);
			} else if (key == "read_log") {
				read_log = static_cast<bool>(value);
			} else if (key == "seed") {
				seed = static_cast<uint32_t>(value);
			} else if (key == "max_depth") {
				max_depth = static_cast<uint32_t>(value);
			} else if (key == "max_width") {
				max_width = static_cast<uint32_t>(value);
			} else if (key == "max_databases") {
				max_databases = static_cast<uint32_t>(value);
			} else if (key == "max_functions") {
				max_functions = static_cast<uint32_t>(value);
			} else if (key == "max_tables") {
				max_tables = static_cast<uint32_t>(value);
			} else if (key == "max_views") {
				max_views = static_cast<uint32_t>(value);
			} else {
				throw std::runtime_error("Unknown option: " + key);
			}
		}
	}

	void GenerateCollationsQuery(std::string &res) {
		const std::filesystem::path &collfile = db_file_path / "collations.data";

		res += "SELECT \"name\" FROM system.collations INTO OUTFILE '";
		res += collfile.generic_string();
		res += "' FORMAT TabSeparated;";
	}

	const std::vector<const std::string> LoadCollations() {
		std::string input;
		std::vector<const std::string> res;
		const std::filesystem::path &collfile = db_file_path / "collations.data";
		std::ifstream infile(collfile);

		input.reserve(64);
		while (std::getline(infile, input)) {
			res.push_back(input);
			input.resize(0);
		}
		return res;
	}
};

}
