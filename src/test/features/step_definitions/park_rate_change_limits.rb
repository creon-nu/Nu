def compact_duration(text)
  case text
  when "6 months" then 18
  when "1 year" then 19
  else
    raise "Unknown duration: #{text.inspect}"
  end
end

def block_duration(text)
  2 ** compact_duration(text)
end

def rate_from_apr(apr, duration)
  apr = parse_number(apr)
  blocks = block_duration(duration)
  blocks_in_year = 60 * 24 * 365.25
  years = blocks.to_f / blocks_in_year
  (apr / 100.0 * years).round(8)
end

def apr_from_rate(rate, duration)
  raise "not rate provided" unless rate
  blocks = block_duration(duration)
  blocks_in_year = 60 * 24 * 365.25
  years = blocks.to_f / blocks_in_year
  (rate / years * 100).round(4)
end

def park_rates_from_table(table)
  table.hashes.map do |row|
    duration = row["Duration"]
    apr = row["Annual percentage rate"]
    {
      block_duration(duration) => rate_from_apr(apr, duration),
    }
  end.inject({}, :merge)
end

When(/^node "(.*?)" votes for these NuBit park rates:$/) do |arg1, table|
  node = @nodes[arg1]
  vote = node.rpc("getvote")
  vote["parkrates"] = [
    {
      "unit" => "B",
      "rates" => park_rates_from_table(table).map do |blocks, rate|
        {
          "blocks" => blocks,
          "rate" => rate,
        }
      end,
    },
  ]
  node.rpc("setvote", vote)
  expect(node.rpc("getvote")["parkrates"]).to eq(vote["parkrates"])
end

When(/^node "(.*?)" finds enough blocks for a park rate vote to become the median$/) do |arg1|
  node = @nodes[arg1]

  height = node.rpc("getblockcount")
  non_vote_weights = (0...5).map do |i|
    node.rpc("getblock", node.rpc("getblockhash", height - i))["coinagedestroyed"]
  end

  vote_weight = 0
  loop do
    block = node.generate_stake
    time_travel(60)

    weight = node.rpc("getblock", block)["coinagedestroyed"]
    vote_weight += weight
    non_vote_weights.shift

    break if vote_weight > non_vote_weights.inject(:+)

    raise "should not happen" if non_vote_weights.empty?
  end
  step "all nodes reach the same height"
end

Then(/^node "(.*?)" finds enough blocks for the APR "(.*?)" to become the median on "(.*?)"$/) do |arg1, arg2, arg3|
  node = @nodes[arg1]
  wait_for do
    result = false
    median_apr = nil
    park_votes = node.rpc("getparkvotes")
    state = park_votes["B"][compact_duration(arg3).to_s]
    if state
      state["votes"].each do |obj|
        if obj["accumulated_percentage"] >= 50
          median_apr = obj["annual_percentage"]
          break
        end
      end
      if median_apr and median_apr == parse_number(arg2)
        result = true
      end
    end

    unless result
      node.generate_stake
      time_travel(60)
    end

    result
  end
  step "all nodes reach the same height"
end

Then(/^the NuBit park rates should be$/) do |table|
  @nodes.values.each do |node|
    node_rates = node.unit_rpc("B", "getparkrates")
    table.hashes.each do |row|
      duration = row["Duration"]
      blocks = block_duration(duration)
      expected_apr = parse_number(row["Annual percentage rate"])
      apr = apr_from_rate(node_rates["#{blocks} blocks"] || 0, row["Duration"])
      expect("#{apr} % on #{duration}").to eq("#{expected_apr} % on #{duration}")
    end
  end
end

