#include "crow.h"

#include <algorithm>
#include <cctype>
#include <mutex>
#include <string>
#include <unordered_set>

namespace {

bool valid_username(const std::string& s)
{
    if (s.empty() || s.size() > 32)
        return false;
    for (unsigned char c : s)
    {
        if (!(std::isalnum(c) != 0 || c == '_'))
            return false;
    }
    return true;
}

std::string trim_copy(std::string s)
{
    auto not_space = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    return s;
}

} // namespace

#ifdef DELETE
constexpr crow::HTTPMethod k_http_post = crow::HTTPMethod::Post;
#else
constexpr crow::HTTPMethod k_http_post = crow::HTTPMethod::POST;
#endif

int main()
{
    crow::SimpleApp app;
    crow::mustache::set_base(CHAT_TEMPLATES_DIR);

    std::mutex clients_mtx;
    std::unordered_set<crow::websocket::connection*> clients;

    CROW_WEBSOCKET_ROUTE(app, "/ws")
        .onaccept([&](const crow::request& req, void** userdata) {
            char* u = req.url_params.get("u");
            if (!u)
                return false;
            std::string name = trim_copy(u);
            if (!valid_username(name))
                return false;
            *userdata = new std::string(std::move(name));
            return true;
        })
        .onopen([&](crow::websocket::connection& conn) {
            std::lock_guard<std::mutex> lock(clients_mtx);
            clients.insert(&conn);
        })
        .onclose([&](crow::websocket::connection& conn, const std::string&) {
            if (auto* p = static_cast<std::string*>(conn.userdata()))
            {
                delete p;
                conn.userdata(nullptr);
            }
            std::lock_guard<std::mutex> lock(clients_mtx);
            clients.erase(&conn);
        })
        .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
            if (is_binary)
                return;
            auto* uname = static_cast<std::string*>(conn.userdata());
            if (!uname)
                return;
            std::string text = trim_copy(data);
            if (text.empty() || text.size() > 2000)
                return;

            crow::json::wvalue msg;
            msg["user"] = *uname;
            msg["text"] = text;
            std::string payload = msg.dump();

            std::lock_guard<std::mutex> lock(clients_mtx);
            for (auto* c : clients)
                c->send_text(payload);
        });

    CROW_ROUTE(app, "/")([] {
        auto page = crow::mustache::load("login.html");
        return page.render();
    });

    CROW_ROUTE(app, "/join").methods(k_http_post)([](const crow::request& req) {
        crow::response res;
        auto params = req.get_body_params();
        char* u = params.get("username");
        if (!u)
        {
            res.code = 303;
            res.set_header("Location", "/");
            return res;
        }
        std::string name = trim_copy(u);
        if (!valid_username(name))
        {
            res.code = 303;
            res.set_header("Location", "/");
            return res;
        }
        res.code = 303;
        res.set_header("Location", std::string("/chat?u=") + name);
        return res;
    });

    CROW_ROUTE(app, "/chat")([](const crow::request& req) -> crow::response {
        char* u = req.url_params.get("u");
        if (!u)
        {
            crow::response r;
            r.code = 303;
            r.set_header("Location", "/");
            return r;
        }
        std::string name = trim_copy(u);
        if (!valid_username(name))
        {
            crow::response r;
            r.code = 303;
            r.set_header("Location", "/");
            return r;
        }

        crow::mustache::context ctx;
        ctx["username"] = name;
        auto page = crow::mustache::load("chat.html");
        crow::response r;
        r.code = 200;
        r.set_header("Content-Type", "text/html; charset=utf-8");
        r.body = page.render(ctx).dump();
        return r;
    });

    app.port(18080).multithreaded().run();
    return 0;
}
