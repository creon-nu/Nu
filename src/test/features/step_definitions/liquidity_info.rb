When(/^node "(.*?)" sends a liquidity of "(.*?)" buy and "(.*?)" sell on unit "(.*?)" from address "(.*?)" with identifier "(.*?)"$/) do |arg1, arg2, arg3, arg4, arg5, arg6|
  node = @nodes[arg1]
  buy = parse_number(arg2)
  sell = parse_number(arg3)
  unit = arg4
  address = @addresses[arg5]
  identifier = arg6
  node.rpc("liquidityinfo", unit, buy, sell, address, identifier)
end

Then(/^node "(.*?)" should reach a total liquidity info of "(.*?)" buy and "(.*?)" sell on unit "(.*?)"$/) do |arg1, arg2, arg3, arg4|
  node = @nodes[arg1]
  buy = parse_number(arg2)
  sell = parse_number(arg3)
  unit = arg4
  wait_for do
    info = node.rpc("getliquidityinfo", unit)
    total = info["total"]
    expect(total["buy"]).to eq(buy)
    expect(total["sell"]).to eq(sell)
  end
end
