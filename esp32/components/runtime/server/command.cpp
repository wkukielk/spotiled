#include "server/command.hpp"
#include "cJSON.h"
#include <memory>

namespace server {

namespace {

struct CJSONDeleter {
    void operator()(cJSON *p) const { cJSON_Delete(p); }
};

using CJSONPtr = std::unique_ptr<cJSON, CJSONDeleter>;

std::optional<Command> parse_fade(const cJSON *root) {
    auto r = cJSON_GetObjectItem(root, "r");
    auto g = cJSON_GetObjectItem(root, "g");
    auto b = cJSON_GetObjectItem(root, "b");
    if (!cJSON_IsNumber(r) || !cJSON_IsNumber(g) || !cJSON_IsNumber(b))
        return std::nullopt;
    return FadeLed{RGB{
        static_cast<uint8_t>(r->valueint),
        static_cast<uint8_t>(g->valueint),
        static_cast<uint8_t>(b->valueint),
    }};
}

std::optional<Command> parse_pulsate(const cJSON *root) {
    auto r = cJSON_GetObjectItem(root, "r");
    auto g = cJSON_GetObjectItem(root, "g");
    auto b = cJSON_GetObjectItem(root, "b");
    auto duration = cJSON_GetObjectItem(root, "duration_ms");
    if (!cJSON_IsNumber(r) || !cJSON_IsNumber(g) || !cJSON_IsNumber(b) ||
        !cJSON_IsNumber(duration) || duration->valueint <= 0)
        return std::nullopt;
    return PulsateLed{RGB{
                          static_cast<uint8_t>(r->valueint),
                          static_cast<uint8_t>(g->valueint),
                          static_cast<uint8_t>(b->valueint),
                      },
                      static_cast<uint32_t>(duration->valueint)};
}

std::optional<Command> parse_rename(const cJSON *root) {
    auto name = cJSON_GetObjectItem(root, "name");
    if (!cJSON_IsString(name) || cJSON_GetStringValue(name)[0] == '\0')
        return std::nullopt;
    return RenameEsp{name->valuestring};
}

} // namespace

std::optional<Command> parse(std::string_view text) {
    CJSONPtr root{cJSON_ParseWithLength(text.data(), text.size())};
    if (!root)
        return std::nullopt;
    auto cmd = cJSON_GetObjectItem(root.get(), "cmd");
    if (!cJSON_IsString(cmd))
        return std::nullopt;
    std::string_view cmd_str{cmd->valuestring};
    if (cmd_str == "fade")
        return parse_fade(root.get());
    if (cmd_str == "pulsate")
        return parse_pulsate(root.get());
    if (cmd_str == "reprovision")
        return Reprovision{};
    if (cmd_str == "rename")
        return parse_rename(root.get());
    if (cmd_str == "force_update")
        return ForceUpdate{};
    return std::nullopt;
}

} // namespace server
