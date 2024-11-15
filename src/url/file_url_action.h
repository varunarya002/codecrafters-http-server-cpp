//
// Created by Varun Arya on 09/11/24.
//

#ifndef FILE_URL_ACTION_H
#define FILE_URL_ACTION_H
#include <filesystem>
#include <fstream>
#include "abstract_url_action.h"
#include "../response/http_response.h"

namespace fs = std::filesystem;

class FileUrlAction: public AbstractUrlAction {
public:
    explicit FileUrlAction(const std::string &resource_name)
        : AbstractUrlAction(resource_name) {
    }

    [[nodiscard]] HttpResponse execute(const HttpRequest &http_request) const override {
        try {
            if (http_request.method == "POST") {
                return executePostRequest(http_request);
            }
            return executeGetRequest(http_request);
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }
private:
    [[nodiscard]] static HttpResponse executeGetRequest(const HttpRequest &http_request) {
        const std:: string filename = http_request.directory_name + http_request.request_param;

        if (doesFileExist(filename)) {
            return returnFileResponse(filename);
        }
        return returnFileNotFindResponse();
    }

    [[nodiscard]] static HttpResponse executePostRequest(const HttpRequest &http_request) {
        const std:: string file = http_request.directory_name + http_request.request_param;

        // Create and write to the file
        std::ofstream outfile(file);
        if (outfile.is_open()) {
            outfile << http_request.body;
            outfile.close();
            std::cout << "File created successfully: " << file << '\n';
        } else {
            std::cerr << "Failed to create the file: " << file << '\n';
        }
        return HttpResponse("Created", 201, "application/octet-stream", 0, "");
    }

    static HttpResponse returnFileResponse(const std::string &filename) {
        const std::string message = "OK";
        const std::string content = readDataFromTheFile(filename);
        return HttpResponse(message, 200, "application/octet-stream", content.length(), content);
    }

    static HttpResponse returnFileNotFindResponse() {
        const std::string message = "Not Found";
        return HttpResponse(message, 404, "application/octet-stream", 0, "");
    }

    static bool doesFileExist(const std::string& filename) {
        return fs::exists(filename);
    }

    static std::string readDataFromTheFile(const std::string &filename) {
        if (!doesFileExist(filename)) {
            throw std::runtime_error("File does not exist!");
        }

        auto fileSize = fs::file_size(filename);

        std::ifstream file(filename, std::ios::in | std::ios::binary);

        if (!file.is_open()) {
            std::cerr << "Error: Could not open the file." << std::endl;
        }

        std::string fileContents(fileSize, '\0');
        file.read(fileContents.data(), fileSize);

        file.close();
        return fileContents;
    }
};



#endif //FILE_URL_ACTION_H
