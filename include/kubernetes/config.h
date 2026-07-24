#pragma once
#include <glaze/glaze.hpp>
#include <optional>
#include <string>
#include <unordered_map>

/*
Rewrite of this file:
https://github.com/kubernetes/client-go/blob/master/tools/clientcmd/api/types.go
*/

namespace kubernetes
{

    template <typename T>
    struct class_from_member_ptr;

    template <typename M, typename C>
    struct class_from_member_ptr<M C::*>
    {
        using type = C;
    };

    template <auto MemPtr>
    constexpr auto omit_false_member()
    {
        // Automatically infer 'Class' from 'MemPtr'
        using Class = typename class_from_member_ptr<decltype(MemPtr)>::type;

        return glz::custom<
            [](Class &obj, bool val) -> void
            {
                obj.*MemPtr = val;
            },
            [](const Class &obj) -> std::optional<bool>
            {
                if (obj.*MemPtr)
                {
                    return true;
                }
                return std::nullopt;
            }>;
    }

    /*
    TODO
    Ai suggests to use std::string_view instead of string, maybe thats optimization worth exploring
    */
    struct Cluster
    {
        // LocationOfOrigin string `json:"-" -> if i will need this i will add this
        std::string server;
        std::optional<std::string> tls_server_name;
        bool insecure_skip_tls_verify;
        std::optional<std::string> certificate_authority{};
        std::optional<std::vector<uint8_t>> certificate_authority_data{};
        std::optional<std::string> proxy_url{};
        bool disable_compression;
        std::optional<std::unordered_map<std::string, glz::generic>> extensions{};
    };

    struct AuthProviderConfig
    {
        std::string name{};

        // map[string]string with `omitempty` translates to an optional unordered_map
        std::optional<std::unordered_map<std::string, std::string>> config{};
    };

    struct ExecEnvVar
    {
        std::string name;
        std::string value;
    };

    using ExecInteractiveMode = std::string;

    struct ExecConfig
    {
        std::string command{};
        std::optional<std::vector<std::string>> args{};
        std::optional<std::vector<ExecEnvVar>> env{};

        std::optional<std::string> api_version{};  // json:"apiVersion,omitempty"
        std::optional<std::string> install_hint{}; // json:"installHint,omitempty"

        bool provide_cluster_info = false;

        // json:"-" fields are excluded from serialization completely
        // (Config, StdinUnavailable, StdinUnavailableMessage, PluginPolicy)

        std::optional<ExecInteractiveMode> interactive_mode{}; // json:"interactiveMode,omitempty"
    };

    struct AuthInfo
    {
        // json:"-" means ignored by serialization (LocationOfOrigin is omitted from meta)
        std::string location_of_origin{};

        std::optional<std::string> client_certificate{};
        std::optional<std::vector<uint8_t>> client_certificate_data{};

        std::optional<std::string> client_key{};
        std::optional<std::vector<uint8_t>> client_key_data{};

        std::optional<std::string> token{};
        std::optional<std::string> token_file{};

        std::optional<std::string> impersonate{};                                                          // json:"act-as,omitempty"
        std::optional<std::string> impersonate_uid{};                                                      // json:"act-as-uid,omitempty"
        std::optional<std::vector<std::string>> impersonate_groups{};                                      // json:"act-as-groups,omitempty"
        std::optional<std::unordered_map<std::string, std::vector<std::string>>> impersonate_user_extra{}; // json:"act-as-user-extra,omitempty"

        std::optional<std::string> username{};
        std::optional<std::string> password{};

        // Pointers handle `omitempty` for nested structs
        std::optional<AuthProviderConfig> auth_provider{};
        std::optional<ExecConfig> exec{};

        // Equivalent to map[string]runtime.Object with `omitempty`
        std::optional<std::unordered_map<std::string, glz::generic>> extensions{};
    };

    struct Context
    {
        std::string location_of_origin{}; // json:"-" (excluded)
        std::string cluster{};
        std::string auth_info{};                                                   // json:"user"
        std::optional<std::string> namespace_{};                                   // json:"namespace,omitempty"
        std::optional<std::unordered_map<std::string, glz::generic>> extensions{}; // json:"extensions,omitempty"
    };

    struct Config
    {
        std::optional<std::string> kind;
        std::optional<std::string> api_version;
        std::unordered_map<std::string, Cluster> clusters{};
        std::unordered_map<std::string, AuthInfo> auth_infos{};
        std::unordered_map<std::string, Context> contexts{};
        std::string current_context;
        std::optional<std::unordered_map<std::string, glz::generic>> extensions{};
    };

}

template <>
struct glz::meta<kubernetes::Config>
{

    using T = kubernetes::Config;

    static constexpr auto value = glz::object(
        "kind", &T::kind,
        "apiVersion", &T::api_version,
        "clusters", &T::clusters,
        "users", &T::auth_infos,
        "contexts", &T::contexts,
        "current-context", &T::current_context,
        "extensions", &T::extensions);
};

template <>
struct glz::meta<kubernetes::Context>
{
    using T = kubernetes::Context;
    constexpr static auto value = object(
        "cluster", &T::cluster,
        "user", &T::auth_info,
        "namespace", &T::namespace_,
        "extensions", &T::extensions);
};

template <>
struct glz::meta<kubernetes::ExecConfig>
{
    using T = kubernetes::ExecConfig;
    constexpr static auto value = object(
        "command", &T::command,
        "args", &T::args,
        "env", &T::env,
        "apiVersion", &T::api_version,
        "installHint", &T::install_hint,
        "provideClusterInfo", &T::provide_cluster_info,
        "interactiveMode", &T::interactive_mode);
};

template <>
struct glz::meta<kubernetes::AuthProviderConfig>
{
    using T = kubernetes::AuthProviderConfig;
    constexpr static auto value = object(
        "name", &T::name,
        "config", &T::config);
};

template <>
struct glz::meta<kubernetes::AuthInfo>
{
    using T = kubernetes::AuthInfo;
    constexpr static auto value = object(
        "client-certificate", &T::client_certificate,
        "client-certificate-data", &T::client_certificate_data,
        "client-key", &T::client_key,
        "client-key-data", &T::client_key_data,
        "token", &T::token,
        "tokenFile", &T::token_file,
        "act-as", &T::impersonate,
        "act-as-uid", &T::impersonate_uid,
        "act-as-groups", &T::impersonate_groups,
        "act-as-user-extra", &T::impersonate_user_extra,
        "username", &T::username,
        "password", &T::password,
        "auth-provider", &T::auth_provider,
        "exec", &T::exec,
        "extensions", &T::extensions);
};

template <>
struct glz::meta<kubernetes::Cluster>
{
    using T = kubernetes::Cluster;
    static constexpr auto value = glz::object(
        "server", &T::server,
        "tls-server-name", &T::tls_server_name,
        "insecure-skip-tls-verify", kubernetes::omit_false_member<&T::insecure_skip_tls_verify>(),
        "certificate-authority", &T::certificate_authority,
        "certificate-authority-data", &T::certificate_authority,
        "proxy-url", &T::proxy_url,
        "disable-compression", kubernetes::omit_false_member<&T::disable_compression>());
};
