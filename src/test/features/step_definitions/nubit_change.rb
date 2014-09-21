Then(/^node "(.*?)" should have (\d+) NuBit transactions$/) do |arg1, arg2|
  @listtransactions = @nodes[arg1].unit_rpc("B", "listtransactions")
  expect(@listtransactions.size).to eq(arg2.to_i)
end

Then(/^the (\d+)\S+ transaction sould be a send of "(.*?)" to "(.*?)"$/) do |arg1, arg2, arg3|
  tx = @listtransactions[arg1.to_i - 1]
  expect(tx["category"]).to eq("send")
  expect(tx["amount"]).to eq(-parse_number(arg2))
  expect(tx["address"]).to eq(@addresses[arg3])
end

