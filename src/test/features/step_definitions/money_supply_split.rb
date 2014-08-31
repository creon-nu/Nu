def unit(unit_name)
  case unit_name
  when "NuShare" then 'S'
  when 'NuBit' then 'B'
  else raise
  end
end

def money_supply(unit_name, node = @nodes.values.first)
  node.unit_rpc(unit(unit_name), "getinfo")["moneysupply"]
end

def parse_number(n)
  n.gsub(',', '').to_f
end

Then(/^the "(.*?)" supply should be between "(.*?)" and "(.*?)"$/) do |arg1, arg2, arg3|
  supply = money_supply(arg1)
  expect(supply).to be >= parse_number(arg2)
  expect(supply).to be <= parse_number(arg3)
end

Then(/^the "(.*?)" supply should be "(.*?)"$/) do |arg1, arg2|
  supply = money_supply(arg1)
  expect(supply).to be == parse_number(arg2)
end

Then(/^"(.*?)" money supply on node "(.*?)" should increase by "(.*?)" when node "(.*?)" finds a block$/) do |arg1, arg2, arg3, arg4|
  unit_name = arg1
  checked_node = @nodes[arg2]
  increase = arg3.to_f
  minter = @nodes[arg4]
  initial_supply = money_supply(unit_name, checked_node)
  minter.generate_stake
  wait_for do
    checked_node.block_count == minter.block_count
  end
  wait_for do
    expect(money_supply(unit_name, checked_node)).to eq(initial_supply + increase)
  end
end

When(/^node "(.*?)" generates a "(.*?)" address "(.*?)"$/) do |arg1, arg2, arg3|
  @addresses[arg3] = @nodes[arg1].unit_rpc(unit(arg2), "getnewaddress")
end

When(/^node "(.*?)" votes an amount of "(.*?)" for custodian "(.*?)"$/) do |arg1, arg2, arg3|
  node = @nodes[arg1]
  vote = node.rpc("getvote")
  vote["custodians"] << {
    "amount" => parse_number(arg2),
    "address" => @addresses[arg3],
  }
  node.rpc("setvote", vote)
end

When(/^node "(.*?)" finds blocks until custodian "(.*?)" is elected$/) do |arg1, arg2|
  node = @nodes[arg1]
  loop do
    block = node.generate_stake
    info = node.rpc("getblock", block)
    if elected_custodians = info["electedcustodians"]
      if elected_custodians.has_key?(@addresses[arg2])
        break
      end
    end
  end
end

