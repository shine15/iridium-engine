//
// Created by Evan Su on 3/5/21.
//

#ifndef IRIDIUM_LIVE_TRADING_EMAIL_HPP_
#define IRIDIUM_LIVE_TRADING_EMAIL_HPP_

#include <string>

namespace iridium {
class EmailClient {
 public:
  EmailClient(
      const std::string &host,
      int port,
      const std::string &username,
      const std::string &password
  );

 private:
  std::string host_;
  int port_;
  std::string username_;
  std::string password_;

};
}  // namespace iridium


#endif //IRIDIUM_LIVE_TRADING_EMAIL_HPP_
