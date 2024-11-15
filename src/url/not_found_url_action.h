#ifndef NOT_FOUND_URL_ACTION_H
#define NOT_FOUND_URL_ACTION_H
#include "abstract_url_action.h"
#include "../response/http_response.h"

class NotFoundUrlAction : public AbstractUrlAction {
public:
    explicit NotFoundUrlAction(const std::string &resource_name)
      : AbstractUrlAction(resource_name) {
    }

    [[nodiscard]] HttpResponse execute(const HttpRequest &http_request) const override {
        const std::string message = "Not Found";
        return HttpResponse(message, 404, "text/plain", 0, "", http_request.headers);
    }
};

#endif //NOT_FOUND_URL_ACTION_H
