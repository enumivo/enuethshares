#include "ex.hpp"

#include <cmath>
#include <enulib/action.hpp>
#include <enulib/asset.hpp>
#include "enu.token.hpp"

using namespace enumivo;
using namespace std;

void ex::receivedenu(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get ENU balance
  double enu_balance = enumivo::token(N(enu.token)).
	   get_balance(N(enu.eth.mm), enumivo::symbol_type(ENU_SYMBOL).name()).amount;
  enu_balance = enu_balance/10000;

  // get ETH balance
  double eth_balance = enumivo::token(N(iou.coin)).
	   get_balance(N(enu.eth.mm), enumivo::symbol_type(ETH_SYMBOL).name()).amount;
  eth_balance = eth_balance/100000000;

  // get total shares
  double shares = enumivo::token(N(shares.coin)).
	   get_supply(enumivo::symbol_type(ENUETH_SYMBOL).name()).amount;
  shares = shares/10000;

  double received = transfer.quantity.amount;
  received = received/10000;

  double a = eth_balance;
  double b = 2 * eth_balance * enu_balance;
  double c = - eth_balance * enu_balance * received;

  double eu = (sqrt((b*b)-(4*a*c)) - b)/(2*a);
  double ee = received - eu;

  double new_shares = shares * (ee/(eu+enu_balance));

  auto quantity = asset(10000*new_shares, ENUETH_SYMBOL);

  action(permission_level{_self, N(active)}, N(shares.coin), N(issue),
         std::make_tuple(transfer.from, quantity,
                         std::string("Issue new ENUETH shares")))
      .send();

  action(permission_level{_self, N(active)}, N(enu.token), N(transfer),
         std::make_tuple(_self, N(enu.eth.mm), transfer.quantity,
                         std::string("Invest ENUETH shares with ENU")))
      .send();
}

void ex::receivedeth(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get ENU balance
  double enu_balance = enumivo::token(N(enu.token)).
	   get_balance(N(enu.eth.mm), enumivo::symbol_type(ENU_SYMBOL).name()).amount;
  enu_balance = enu_balance/10000;

  // get ETH balance
  double eth_balance = enumivo::token(N(iou.coin)).
	   get_balance(N(enu.eth.mm), enumivo::symbol_type(ETH_SYMBOL).name()).amount;
  eth_balance = eth_balance/100000000;

  // get total shares
  double shares = enumivo::token(N(shares.coin)).
	   get_supply(enumivo::symbol_type(ENUETH_SYMBOL).name()).amount;
  shares = shares/10000;

  double received = transfer.quantity.amount;
  received = received/100000000;

  double a = enu_balance;
  double b = 2 * enu_balance * eth_balance;
  double c = - enu_balance * eth_balance * received;

  double ue = (sqrt((b*b)-(4*a*c)) - b)/(2*a);
  double uu = received - ue;

  double new_shares = shares * (uu/(ue+eth_balance));

  auto quantity = asset(10000*new_shares, ENUETH_SYMBOL);

  action(permission_level{_self, N(active)}, N(shares.coin), N(issue),
         std::make_tuple(transfer.from, quantity,
                         std::string("Issue new ENUETH shares")))
      .send();

  action(permission_level{_self, N(active)}, N(iou.coin), N(transfer),
         std::make_tuple(_self, N(enu.eth.mm), transfer.quantity,
                         std::string("Invest ENUETH shares with ENU")))
      .send();
}

void ex::receivedshares(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get ENU balance
  double enu_balance = enumivo::token(N(enu.token)).
	   get_balance(N(enu.eth.mm), enumivo::symbol_type(ENU_SYMBOL).name()).amount;
  enu_balance = enu_balance/10000;

  // get ETH balance
  double eth_balance = enumivo::token(N(iou.coin)).
	   get_balance(N(enu.eth.mm), enumivo::symbol_type(ETH_SYMBOL).name()).amount;
  eth_balance = eth_balance/100000000;

  // get total shares
  double shares = enumivo::token(N(shares.coin)).
	   get_supply(enumivo::symbol_type(ENUETH_SYMBOL).name()).amount;
  shares = shares/10000;

  double received = transfer.quantity.amount;
  received = received/10000;

  double ethportion = eth_balance*(received/shares);
  auto eth = asset(100000000*ethportion, ETH_SYMBOL);

  action(permission_level{N(enu.eth.mm), N(active)}, N(iou.coin), N(transfer),
         std::make_tuple(N(enu.eth.mm), transfer.from, eth,
                         std::string("Divest ENUETH shares for ETH")))
      .send();

  double enuportion = enu_balance*(received/shares);
  auto enu = asset(10000*enuportion, ENU_SYMBOL);

  action(permission_level{N(enu.eth.mm), N(active)}, N(enu.token), N(transfer),
         std::make_tuple(N(enu.eth.mm), transfer.from, enu,
                         std::string("Divest ENUETH shares for ENU")))
      .send();

  action(permission_level{_self, N(active)}, N(shares.coin), N(retire),
         std::make_tuple(transfer.quantity, std::string("Retire ENUETH shares")))
      .send();
}

void ex::apply(account_name contract, action_name act) {

  if (contract == N(enu.token) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();

    enumivo_assert(transfer.quantity.symbol == ENU_SYMBOL,
                 "Must send ENU");
    receivedenu(transfer);
    return;
  }

  if (contract == N(iou.coin) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();

    enumivo_assert(transfer.quantity.symbol == ETH_SYMBOL,
                 "Must send ETH");
    receivedeth(transfer);
    return;
  }

  if (contract == N(shares.coin) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();

    enumivo_assert(transfer.quantity.symbol == ENUETH_SYMBOL,
                 "Must send ENUETH");
    receivedshares(transfer);
    return;
  }

  if (act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();
    enumivo_assert(false, "Must send ETH or ENU or ENUETH");
    return;
  }

  if (contract != _self) return;

}

extern "C" {
[[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
  ex enuethshares(receiver);
  enuethshares.apply(code, action);
  enumivo_exit(0);
}
}
