// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/bigtable/admin_client.h"
#include <gmock/gmock.h>

namespace bigtable = google::cloud::bigtable;

TEST(AdminClientTest, Default) {
  auto admin_client = bigtable::CreateDefaultAdminClient(
      "test-project", bigtable::ClientOptions().set_connection_pool_size(1));
  ASSERT_TRUE(admin_client);
  EXPECT_EQ("test-project", admin_client->project());

  auto channel0 = admin_client->Channel();
  EXPECT_TRUE(channel0);

  auto channel1 = admin_client->Channel();
  EXPECT_EQ(channel0.get(), channel1.get());

  admin_client->reset();
  channel1 = admin_client->Channel();
  EXPECT_TRUE(channel1);
  EXPECT_NE(channel0.get(), channel1.get());
}
