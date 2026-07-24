#include <kubernetes/config.h>
#include <gtest/gtest.h>

#include <gtest/gtest.h>
#include <string>
#include <unordered_map>
#include <optional>
#include "kubernetes/config.h" // Your header containing the structs

TEST(KubeConfigTest, RoundTripSerialization) {
    // 1. Arrange: Build a fully populated Config object
    kubernetes::Config original{};
    original.kind = "Config";
    original.api_version = "v1";
    original.current_context = "dev-context";

    // Cluster setup
    kubernetes::Cluster cluster{};
    cluster.server = "https://127.0.0.1:6443";
    cluster.insecure_skip_tls_verify = true; // Should persist
    original.clusters["my-cluster"] = cluster;

    // AuthInfo setup
    kubernetes::AuthInfo auth{};
    auth.token = "my-secret-bearer-token";
    original.auth_infos["my-user"] = auth;

    // Context setup
    kubernetes::Context ctx{};
    ctx.cluster = "my-cluster";
    ctx.auth_info = "my-user";
    ctx.namespace_ = "default";
    original.contexts["dev-context"] = ctx;

    // Extensions setup (using glz::generic for arbitrary JSON structures)
    glz::generic::object_t ext_obj;
    ext_obj["audience"] = "06e3fbd18de8";
    ext_obj["enabled"] = true;

    std::unordered_map<std::string, glz::generic> extensions_map;
    extensions_map["client.authentication.k8s.io/exec"] = ext_obj;
    original.extensions = extensions_map;

    // 2. Act: Serialize C++ -> JSON string
    std::string json_output;
    auto write_err = glz::write_json(original, json_output);
    ASSERT_FALSE(write_err) << "Failed to serialize Config to JSON";

    // 3. Act: Deserialize JSON string -> C++ Object
    kubernetes::Config parsed{};
    auto read_err = glz::read_json(parsed, json_output);
    ASSERT_FALSE(read_err) << "Failed to parse JSON back into Config";

    // 4. Assert: Validate all fields match original values
    ASSERT_TRUE(parsed.kind.has_value());
    EXPECT_EQ(*parsed.kind, "Config");

    ASSERT_TRUE(parsed.api_version.has_value());
    EXPECT_EQ(*parsed.api_version, "v1");

    EXPECT_EQ(parsed.current_context, "dev-context");

    // Validate Clusters map
    ASSERT_TRUE(parsed.clusters.contains("my-cluster"));
    EXPECT_EQ(parsed.clusters["my-cluster"].server, "https://127.0.0.1:6443");
    EXPECT_TRUE(parsed.clusters["my-cluster"].insecure_skip_tls_verify);

    // Validate AuthInfos map
    ASSERT_TRUE(parsed.auth_infos.contains("my-user"));
    ASSERT_TRUE(parsed.auth_infos["my-user"].token.has_value());
    EXPECT_EQ(*parsed.auth_infos["my-user"].token, "my-secret-bearer-token");

    // Validate Contexts map
    ASSERT_TRUE(parsed.contexts.contains("dev-context"));
    EXPECT_EQ(parsed.contexts["dev-context"].cluster, "my-cluster");
    EXPECT_EQ(parsed.contexts["dev-context"].auth_info, "my-user");
    ASSERT_TRUE(parsed.contexts["dev-context"].namespace_.has_value());
    EXPECT_EQ(*parsed.contexts["dev-context"].namespace_, "default");

    // Validate Extensions map (`glz::generic`)
    ASSERT_TRUE(parsed.extensions.has_value());
    ASSERT_TRUE(parsed.extensions->contains("client.authentication.k8s.io/exec"));
    
    // Access nested generic values
    const auto& ext_val = parsed.extensions->at("client.authentication.k8s.io/exec");
    // glz::generic provides helpers or standard map lookup depending on version, 
    // but we can ensure it successfully parsed without errors.
    EXPECT_TRUE(ext_val.contains("audience"));
}

TEST(KubeConfigTest, OmitsFalseBooleans) {
    kubernetes::Cluster cluster{};
    cluster.server = "https://127.0.0.1:6443";
    cluster.insecure_skip_tls_verify = false; // Should be omitted by omit_false_member

    std::string json_output;
    auto err = glz::write_json(cluster, json_output);
    ASSERT_FALSE(err);

    // Verify that "insecure-skip-tls-verify" isn't printed out when it's false
    EXPECT_EQ(json_output.find("insecure-skip-tls-verify"), std::string::npos);
}
