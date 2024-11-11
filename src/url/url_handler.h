#ifndef URL_HANDLER_H
#define URL_HANDLER_H
#include <memory>
#include <regex>
#include <string>

#include "abstract_url_action.h"
#include "not_found_url_action.h"
#include "../request/http_request.h"

struct HttpRequest;

class URLHandler {
public:
    URLHandler() = default;

    void registerUrl(const std::string& url_name, std::shared_ptr<AbstractUrlAction> action) {
        url_map[url_name] = std::move(action);
    }

    [[nodiscard]] std::string sendResponseForUrl(const HttpRequest &http_request, const std::string& directory_name) const {
        std::string url_path = http_request.path;
        if (url_path[url_path.size()-1] != '/') {
            url_path.push_back('/');
        }

        for (auto &it: url_map) {
            std::string pattern = "^" + it.first + "/(.*)";
            std::regex regex_pattern(pattern);
            std::smatch matches;
            if (std::regex_search(url_path, matches, regex_pattern)) {

                HttpRequest http_request_with_params = HttpRequest();
                http_request_with_params.method = http_request.method;
                http_request_with_params.path = http_request.path;
                std::string param = matches[1];
                if (param.ends_with("/")) {
                    param.pop_back();
                }
                http_request_with_params.request_param = param;
                http_request_with_params.headers = http_request.headers;
                http_request_with_params.directory_name = directory_name;

                return it.second->execute(http_request_with_params).sendResponse();
            }
        }
        return NotFoundUrlAction("404").execute(http_request).sendResponse();
    }
private:
    std::pmr::unordered_map<std::string, std::shared_ptr<AbstractUrlAction>> url_map;
};

#endif //URL_HANDLER_H
