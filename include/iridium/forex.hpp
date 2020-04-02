/* Copyright 2020 Iridium. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef INCLUDE_IRIDIUM_FOREX_HPP_
#define INCLUDE_IRIDIUM_FOREX_HPP_

#include <math.h>
#include <stdlib.h>

namespace iridium {
    /*
     * https://www.fxpro.com/trading-tools/calculators/pip
     * @param units
     * @param rate: account currency vs quote currency rate
     * @param decimals: pip decimals
     */
    double CalculatePipValue(int units, double rate, int decimals);

    /*
     * https://www.oanda.com/forex-trading/analysis/profit-calculator/
     * @param change: Pip change
     * @param units
     * @param rate: Account currency vs quote currency rate
     * @param decimals: pip decimals
     */
    double CalculateGainsLosses(
        double change,
        int units,
        double rate,
        int decimals);

    /*
     *  https://www.oanda.com/resources/legal/united-states/legal/margin-rules
     *  @param units
     *  @param rate: account currency vs base currency rate
     *  @param leverage
     */
    double CalculateMarginUsed(int units, double rate, int leverage);

    /*
     * @param nav: net asset value
     * @param margin_used
     */
    double CalculateMarginAvailable(double nav, double margin_used);

    /*
     * https://www.oanda.com/resources/legal/australia/legal/margin-rules
     * A margin closeout will be triggered when the Margin Closeout Value declines to half, or less than half, of the Margin Used
     * @param nav: net asset value
     * @param margin_used
     */
    bool CheckMarginCall(double nav, double margin_used);

    /*
     * Calculate position size using stop loss
     * https://www.babypips.com/tools/position-size-calculator
     * Balance is the figure of the account which includes all closed trades. Equity is the actual amount of funds in the
     * account currently including all open trades.
     * @param equity
     * @param risk_pct: risk percentage
     * @param stop_loss_pips
     * @param rate: Account currency vs quote currency rate
     * @param pip_num: pip number
     */
    int CalculatePositionSize(
        double equity,
        double risk_pct,
        int stop_loss_pips,
        double rate,
        int pip_num);

    /*
     * @param size
     * @param current_price
     * @param rate: Account currency vs quote currency rate
     */
    double CalculatePositionValue(int size, double current_price, double rate);

}  // namespace iridium

#endif  // INCLUDE_IRIDIUM_FOREX_HPP_
