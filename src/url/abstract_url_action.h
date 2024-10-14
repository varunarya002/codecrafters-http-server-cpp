#ifndef ABSTRACT_URL_ACTION_H
#define ABSTRACT_URL_ACTION_H
#include <string>

struct HttpRequest;

class AbstractUrlAction {
public:
    virtual ~AbstractUrlAction() = default;
    explicit AbstractUrlAction(const std::string &resource_name): resource_name(resource_name) {};
    [[nodiscard]] virtual std::string execute(const HttpRequest &http_request) const = 0;
protected:
    std::string resource_name;
};

#endif //ABSTRACT_URL_ACTION_H
