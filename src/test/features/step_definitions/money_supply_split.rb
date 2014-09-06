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

