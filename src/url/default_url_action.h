#ifndef DEFAULT_URL_ACTION_H
#define DEFAULT_URL_ACTION_H
#include <string>

#include "abstract_url_action.h"
#include "../response/http_response.h"

struct HttpRequest;

class DefaultUrlAction : public AbstractUrlAction {
public:
    explicit DefaultUrlAction(const std::string &resource_name)
      : AbstractUrlAction(resource_name) {
    }

    [[nodiscard]] HttpResponse execute(const HttpRequest &http_request) const override {
        const std::string message = "OK";
        return HttpResponse(message, 200, "text/plain", 0, "");
    }
};

#endif //DEFAULT_URL_ACTION_H
