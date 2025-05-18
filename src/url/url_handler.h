#ifndef URL_HANDLER_H
#define URL_HANDLER_H
#include <memory>
#include <regex>
#include <string>

#include "abstract_url_action.h"
#include "not_found_url_action.h"
#include "../request/http_request.h"

// Forward declaration
struct HttpRequest;

/**
 * @class URLHandler
 * @brief Handles URL routing by matching request paths to registered actions
 */
class URLHandler {
public:
    URLHandler() = default;

    /**
     * @brief Register a URL pattern with an action handler
     * @param url_name URL pattern to match
     * @param action Action to execute when URL matches
     */
    void registerUrl(const std::string& url_name, std::shared_ptr<AbstractUrlAction> action) {
        url_map[url_name] = std::move(action);
    }

    /**
     * @brief Process an HTTP request and produce a response
     * @param http_request The incoming HTTP request
     * @param directory_name Base directory for file operations
     * @return Response string to send back to client
     */
    [[nodiscard]] std::string sendResponseForUrl(const HttpRequest &http_request, const std::string& directory_name) const {
        // Normalize path by ensuring it ends with a slash
        std::string url_path = http_request.path;
        if (url_path.empty() || url_path.back() != '/') {
            url_path.push_back('/');
        }

        // Try to match the URL against registered patterns
        for (const auto &[pattern_key, action] : url_map) {
            // Create regex pattern to extract parameters
            const std::string regex_str = "^" + pattern_key + "/(.*)";
            std::regex regex_pattern(regex_str);
            std::smatch matches;

            if (std::regex_search(url_path, matches, regex_pattern)) {
                // Match found - prepare request with parameters
                HttpRequest http_request_with_params = http_request;

                // Extract and process parameter
                std::string param = matches[1];
                if (!param.empty() && param.back() == '/') {
                    param.pop_back();
                }

                // Update request with extracted parameters
                http_request_with_params.request_param = param;
                http_request_with_params.directory_name = directory_name;

                // Execute the matched action and return response
                return action->execute(http_request_with_params).sendResponse();
            }
        }

        // No match found - return 404 Not Found
        return NotFoundUrlAction("404").execute(http_request).sendResponse();
    }

private:
    /** Map of URL patterns to their handler actions */
    std::pmr::unordered_map<std::string, std::shared_ptr<AbstractUrlAction>> url_map;
};

#endif //URL_HANDLER_H
