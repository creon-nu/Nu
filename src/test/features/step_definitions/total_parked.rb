Then(/^the total parked NuBits should reach "(.*?)"$/) do |arg1|
  wait_for do
    expected = Array.new(@nodes.size, parse_number(arg1))
    actual = @nodes.values.map { |node| node.unit_rpc("B", "getinfo")["totalparked"] }
    expect(actual).to eq(expected)
  end
end
