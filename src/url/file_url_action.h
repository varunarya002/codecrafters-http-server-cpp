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
        const std:: string filename = http_request.directory_name + http_request.request_param;

        if (doesFileExist(filename)) {
            return returnFileResponse(filename);
        }
        return returnFileNotFindResponse();
    }
private:
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
