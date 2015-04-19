Before do
  @blocks = {}
  @addresses = {}
  @nodes = {}
  @tx = {}
  @raw_tx = {}
  @raw_tx_complete = {}
  @pubkeys = {}
  @unit = {}
  @time = Time.utc(2014, 9, 1, 0, 0, 0)
end

def timeshift
  (@time - Time.now).to_i
end

def time_travel(seconds)
  @time += seconds
  @nodes.values.each do |node|
    node.rpc "timetravel", seconds
  end
end

Given(/^a network with nodes? (.+)(?: able to mint)?$/) do |node_names|
  node_names = node_names.scan(/"(.*?)"/).map(&:first)
  available_nodes = %w( a b c d e )
  raise "More than #{available_nodes.size} nodes not supported" if node_names.size > available_nodes.size
  @nodes = {}

  node_names.each_with_index do |name, i|
    options = {
      name: name,
      image: "nunet/#{available_nodes[i]}",
      links: @nodes.values.map(&:name),
      args: {
        debug: true,
        timetravel: timeshift,
      },
    }
    node = CoinContainer.new(options)
    @nodes[name] = node
    node.wait_for_boot
  end

  wait_for(10) do
    @nodes.values.all? do |node|
      count = node.connection_count
      count == @nodes.size - 1
    end
  end
  wait_for do
    @nodes.values.map do |node|
      count = node.block_count
      count
    end.uniq.size == 1
  end
end

Given(/^a node "(.*?)" with an empty wallet$/) do |arg1|
  name = arg1
  options = {
    name: name,
    image: "nunet/empty",
    links: @nodes.values.map(&:name),
    args: {
      debug: true,
      timetravel: timeshift,
    },
  }
  node = CoinContainer.new(options)
  @nodes[name] = node
  node.wait_for_boot
end

Given(/^a node "(.*?)" connected only to node "(.*?)"$/) do |arg1, arg2|
  other_node = @nodes[arg2]
  name = arg1
  options = {
    image: "nunet/a",
    links: [other_node.name],
    link_with_connect: true,
    args: {
      debug: true,
      timetravel: timeshift,
    },
    remove_wallet_before_startup: true,
  }
  node = CoinContainer.new(options)
  @nodes[name] = node
  node.wait_for_boot
end

Given(/^a node "([^"]*?)"$/) do |arg1|
  step "a node \"#{arg1}\" with an empty wallet"
end

Given(/^a node "(.*?)" with an empty wallet and with avatar mode disabled$/) do |arg1|
  name = arg1
  options = {
    name: name,
    image: "nunet/empty",
    links: @nodes.values.map(&:name),
    link_with_connect: true,
    args: {
      debug: true,
      timetravel: timeshift,
      avatar: false,
    },
  }
  node = CoinContainer.new(options)
  @nodes[name] = node
  node.wait_for_boot
end

Given(/^a node "(.*?)" running version "(.*?)"$/) do |arg1, arg2|
  name = arg1
  options = {
    name: name,
    image: "nunet/#{arg2}",
    bind_code: false,
    links: @nodes.values.map(&:name),
    args: {
      debug: true,
      timetravel: timeshift,
    },
    remove_wallet_before_startup: true,
  }
  begin
    node = CoinContainer.new(options)
  rescue Docker::Error::NotFoundError
    puts "Image for version #{arg2.inspect} may be missing."
    raise
  end
  @nodes[name] = node
  node.wait_for_boot
end

Given(/^a node "(.*?)" running version "(.*?)" and able to mint$/) do |arg1, arg2|
  name = arg1
  options = {
    name: name,
    image: "nunet/#{arg2}",
    bind_code: false,
    links: @nodes.values.map(&:name),
    args: {
      debug: true,
      timetravel: timeshift,
    },
  }
  begin
    node = CoinContainer.new(options)
  rescue Docker::Error::NotFoundError
    puts "Image for version #{arg2.inspect} may be missing."
    raise
  end
  @nodes[name] = node
  node.wait_for_boot
end

Given(/^a node "(.*?)" only connected to node "(.*?)"$/) do |arg1, arg2|
  name = arg1
  options = {
    name: name,
    image: "nunet/empty",
    links: [@nodes[arg2]].map(&:name),
    link_method: "connect",
    args: {
      debug: true,
      timetravel: timeshift,
    },
  }
  node = CoinContainer.new(options)
  @nodes[name] = node
  node.wait_for_boot
end

After do
  if @nodes
    require 'thread'
    @nodes.values.reverse.map do |node|
      Thread.new do
        node.shutdown
        #node.wait_for_shutdown
        #begin
        #  node.container.delete(force: true)
        #rescue
        #end
      end
    end.each(&:join)
  end
end

When(/^node "(.*?)" restarts$/) do |arg1|
  @nodes[arg1].tap do |node|
    node.shutdown
    node.wait_for_shutdown
    node.start
    node.wait_for_boot
  end
end

When(/^node "(.*?)" finds a block "([^"]*?)"$/) do |node, block|
  time_travel(5)
  @blocks[block] = @nodes[node].generate_stake
end

When(/^node "(.*?)" finds a block "([^"]*?)" not received by node "([^"]*?)"$/) do |node, block, other|
  time_travel(5)
  @nodes[other].rpc("ignorenextblock")
  @blocks[block] = @nodes[node].generate_stake
end

When(/^node "(.*?)" finds a block$/) do |node|
  time_travel(5)
  @nodes[node].generate_stake
end

When(/^node "(.*?)" finds (\d+) blocks$/) do |arg1, arg2|
  time_travel(5)
  arg2.to_i.times do
    @nodes[arg1].generate_stake
    time_travel(60)
  end
end

When(/^node "(.*?)" finds (\d+|a) blocks? received by all (?:nodes|other nodes)$/) do |arg1, arg2|
  step "node \"#{arg1}\" finds #{arg2 == "a" ? 1 : arg2} blocks"
  step "all nodes reach the same height"
end

Then(/^all nodes should (?:be at|reach) block "(.*?)"$/) do |block|
  begin
    wait_for do
      main = @nodes.values.map(&:top_hash)
      main.all? { |hash| hash == @blocks[block] }
    end
  rescue
    require 'pp'
    pp @blocks
    raise "Not at block #{block}: #{@nodes.values.map(&:top_hash).map { |hash| @blocks.key(hash) || hash }.inspect}"
  end
end

Then(/^nodes? (.+) (?:should be at|should reach|reach|reaches|is at|are at) block "(.*?)"$/) do |node_names, block|
  nodes = node_names.scan(/"(.*?)"/).map { |name, | @nodes[name] }
  begin
    wait_for do
      main = nodes.map(&:top_hash)
      main.all? { |hash| hash == @blocks[block] }
    end
  rescue
    raise "Not at block #{block}: #{nodes.map(&:top_hash).map { |hash| @blocks.key(hash) || hash }.inspect}"
  end
end

When(/^node "(.*?)" reaches block "(.*?)"$/) do |arg1, arg2|
  node = @nodes[arg1]
  block = @blocks[arg2]
  begin
    wait_for do
      expect(node.top_hash).to eq(block)
    end
  rescue Exception
    p @blocks
    raise
  end
end

Then(/^node "(.*?)" should stay at block "(.*?)"$/) do |arg1, arg2|
  node = @nodes[arg1]
  block = @blocks[arg2]
  expect(node.top_hash).to eq(block)
  sleep 2
  expect(node.top_hash).to eq(block)
end

Given(/^all nodes (?:should )?reach the same height$/) do
  wait_for do
    expect(@nodes.values.map(&:block_count).uniq.size).to eq(1)
  end
end

Given(/^all nodes reach block "(.*?)"$/) do |arg1|
  step "all nodes should be at block \"#{arg1}\""
end

When(/^node "(.*?)" votes an amount of "(.*?)" for custodian "(.*?)"$/) do |arg1, arg2, arg3|
  node = @nodes[arg1]
  vote = node.rpc("getvote")
  vote["custodians"] << {
    "address" => @addresses[arg3],
    "amount" => parse_number(arg2),
  }
  result = node.rpc("setvote", vote)
  expect(result).to eq(vote)
end

When(/^node "(.*?)" votes a park rate of "(.*?)" NuBits per Nubit parked during (\d+) blocks$/) do |arg1, arg2, arg3|
  node = @nodes[arg1]
  vote = node.rpc("getvote")
  vote["parkrates"] = [
    {
      "unit" => "B",
      "rates" => [
        {
          "blocks" => arg3.to_i,
          "rate" => parse_number(arg2),
        },
      ],
    },
  ]
  node.rpc("setvote", vote)
  expect(node.rpc("getvote")["parkrates"]).to eq(vote["parkrates"])
end

When(/^node "(.*?)" finds blocks until custodian "(.*?)" is elected$/) do |arg1, arg2|
  node = @nodes[arg1]
  wait_for do
    done = false
    block = node.generate_stake
    time_travel(60)
    info = node.rpc("getblock", block)
    if elected_custodians = info["electedcustodians"]
      if elected_custodians.has_key?(@addresses[arg2])
        done = true
      end
    end
    done
  end
end

When(/^node "(.*?)" finds blocks until the NuBit park rate for (\d+) blocks is "(.*?)"$/) do |arg1, arg2, arg3|
  node = @nodes[arg1]
  wait_for do
    block = node.generate_stake
    time_travel(60)
    info = node.rpc("getblock", block)
    park_rates = info["parkrates"].detect { |r| r["unit"] == "B" }
    expect(park_rates).not_to be_nil
    rates = park_rates["rates"]
    rate = rates.detect { |r| r["blocks"] == arg2.to_i }
    expect(rate).not_to be_nil
    expect(rate["rate"]).to eq(parse_number(arg3))
  end
end

When(/^node "(.*?)" finds blocks until custodian "(.*?)" is elected in transaction "(.*?)"$/) do |arg1, arg2, arg3|
  node = @nodes[arg1]
  address = @addresses[arg2]
  wait_for do
    done = false
    block = node.generate_stake
    time_travel(60)
    info = node.rpc("getblock", block)
    if elected_custodians = info["electedcustodians"]
      if elected_custodians.has_key?(address)
        info["tx"].each do |txid|
          tx = node.rpc("getrawtransaction", txid, 1)
          tx["vout"].each do |out|
            if out["scriptPubKey"]["addresses"] == [address]
              @tx[arg3] = txid
              done = true
              break
            end
          end
        end
        raise "Custodian grant transaction not found" if @tx[arg3].nil?
        break
      end
    end
    done
  end
end

When(/^node "(.*?)" sends "(.*?)" to "([^"]*?)" in transaction "(.*?)"$/) do |arg1, arg2, arg3, arg4|
  @tx[arg4] = @nodes[arg1].rpc "sendtoaddress", @addresses.fetch(arg3, arg3), parse_number(arg2)
end

When(/^node "(.*?)" sends "(.*?)" to "([^"]*?)"$/) do |arg1, arg2, arg3|
  @nodes[arg1].rpc "sendtoaddress", @addresses.fetch(arg3, arg3), parse_number(arg2)
end

When(/^node "(.*?)" sends "(.*?)" (NuBits|NBT|NuShares|NSR) to "(.*?)"$/) do |arg1, arg2, unit_name, arg3|
  @nodes[arg1].unit_rpc unit(unit_name), "sendtoaddress", @addresses.fetch(arg3, arg3), parse_number(arg2)
end

Given(/^node "(.*?)" sends "(.*?)" (\w+) to node "(.*?)"$/) do |arg1, arg2, arg3, arg4|
  node = @nodes[arg1]
  amount = parse_number(arg2)
  unit = unit(arg3)
  target_node = @nodes[arg4]

  target_address = target_node.unit_rpc(unit, "getaccountaddress", "")
  node.unit_rpc(unit, "sendtoaddress", target_address, amount)
end

def debug_balance(node, unit_name)
  require 'pp'
  pp(
    unconfirmed_balance: node.unit_rpc(unit(unit_name), "getbalance", "*", 0),
    balance: node.unit_rpc(unit(unit_name), "getbalance"),
    balance_all_accounts: node.unit_rpc(unit(unit_name), "getbalance", "*"),
  )
end

Then(/^node "(.*?)" (?:should reach|reaches) a balance of "([^"]*?)"( NuBits| NuShares|)$/) do |arg1, arg2, unit_name|
  node = @nodes[arg1]
  amount = parse_number(arg2)
  begin
    wait_for do
      expect(node.unit_rpc(unit(unit_name), "getbalance")).to eq(amount)
      expect(node.unit_rpc(unit(unit_name), "getbalance", "*")).to eq(amount)
    end
  rescue RSpec::Expectations::ExpectationNotMetError
    debug_balance(node, unit_name)
    raise
  end
end

Then(/^node "(.*?)" should have a balance of "([^"]*?)"( NuBits|)$/) do |arg1, arg2, unit_name|
  node = @nodes[arg1]
  amount = parse_number(arg2)
  begin
    wait_for do
      expect(node.unit_rpc(unit(unit_name), "getbalance")).to eq(amount)
      expect(node.unit_rpc(unit(unit_name), "getbalance", "*")).to eq(amount)
    end
  rescue RSpec::Expectations::ExpectationNotMetError
    debug_balance(node, unit_name)
    raise
  end
end

Then(/^node "(.*?)" should reach an unconfirmed balance of "([^"]*?)"( NuBits|)$/) do |arg1, arg2, unit_name|
  node = @nodes[arg1]
  amount = parse_number(arg2)
  wait_for do
    expect(node.unit_rpc(unit(unit_name), "getbalance", "*", 0)).to eq(amount)
  end
end

Then(/^node "(.*?)" should have an unconfirmed balance of "([^"]*?)"( NuBits|)$/) do |arg1, arg2, unit_name|
  node = @nodes[arg1]
  amount = parse_number(arg2)
  expect(node.unit_rpc(unit(unit_name), "getbalance", "*", 0)).to eq(amount)
end

Then(/^node "(.*?)" should reach a balance of "([^"]*?)"( NuBits|) on account "([^"]*?)"$/) do |arg1, arg2, unit_name, account|
  node = @nodes[arg1]
  amount = parse_number(arg2)
  wait_for do
    expect(node.unit_rpc(unit(unit_name), "getbalance", account)).to eq(amount)
  end
end

Given(/^node "(.*?)" generates a (\w+) address "(.*?)"$/) do |arg1, unit_name, arg2|
  unit_name = "NuShares" if unit_name == "new"
  @addresses[arg2] = @nodes[arg1].unit_rpc(unit(unit_name), "getnewaddress")
  @unit[@addresses[arg2]] = unit(unit_name)
end

When(/^node "(.*?)" sends "(.*?)" shares to "(.*?)" through transaction "(.*?)"$/) do |arg1, arg2, arg3, arg4|
  @tx[arg4] = @nodes[arg1].rpc "sendtoaddress", @addresses[arg3], arg2.to_f
end

Then(/^transaction "(.*?)" on node "(.*?)" should have (\d+) confirmations?$/) do |arg1, arg2, arg3|
  wait_for do
    expect(@nodes[arg2].rpc("gettransaction", @tx[arg1])["confirmations"]).to eq(arg3.to_i)
  end
end

Then(/^all nodes should (?:have|reach) (\d+) transactions? in memory pool$/) do |arg1|
  wait_for do
    expect(@nodes.values.map { |node| node.rpc("getmininginfo")["pooledtx"] }).to eq(@nodes.map { arg1.to_i })
  end
end

Then(/^node "(.*?)" (?:should have|should reach|reaches) (\d+) transactions? in memory pool$/) do |arg1, arg2|
  node = @nodes[arg1]
  wait_for do
    expect(node.rpc("getmininginfo")["pooledtx"]).to eq(arg2.to_i)
  end
end

Then(/^the NuBit balance of node "(.*?)" should reach "(.*?)"$/) do |arg1, arg2|
  wait_for do
    expect(@nodes[arg1].unit_rpc('B', 'getbalance')).to eq(parse_number(arg2))
  end
end

When(/^some time pass$/) do
  time_travel(5)
end

When(/^node "(.*?)" finds enough blocks to mature a Proof of Stake block$/) do |arg1|
  node = @nodes[arg1]
  3.times do
    node.generate_stake
    time_travel(60)
  end
end

When(/^node "(.*?)" parks "(.*?)" NuBits (?:for|during) (\d+) blocks$/) do |arg1, arg2, arg3|
  time_travel(5)
  node = @nodes[arg1]
  amount = parse_number(arg2)
  blocks = arg3.to_i

  node.unit_rpc('B', 'park', amount, blocks)
end

When(/^node "(.*?)" parks "(.*?)" NuBits (?:for|during) (\d+) blocks with "(.*?)" as unpark address$/) do |arg1, arg2, arg3, arg4|
  time_travel(5)
  node = @nodes[arg1]
  amount = parse_number(arg2)
  blocks = arg3.to_i
  unpark_address = @addresses[arg4]

  node.unit_rpc('B', 'park', amount, blocks, "", unpark_address)
end

When(/^node "(.*?)" unparks$/) do |arg1|
  time_travel(5)
  node = @nodes[arg1]
  node.unit_rpc('B', 'unpark')
end

When(/^node "(.*?)" unparks to transaction "(.*?)"$/) do |arg1, arg2|
  time_travel(5)
  node = @nodes[arg1]
  txs = node.unit_rpc('B', 'unpark')
  raise "No unpark transaction received" if txs.empty?
  raise "multiple unparks" if txs.size > 1
  @tx[arg2] = txs.first
end

Then(/^(?:node |)"(.*?)" should have "(.*?)" NuBits parked$/) do |arg1, arg2|
  node = @nodes[arg1]
  amount = parse_number(arg2)
  info = node.unit_rpc("B", "getinfo")
  wait_for do
    expect(info["parked"]).to eq(amount)
  end
end

When(/^the nodes travel to the Nu protocol v(\d+) switch time$/) do |arg1|
  switch_time = Time.at(1414195200)
  @nodes.values.each do |node|
    time = Time.parse(node.info["time"])
    node.rpc("timetravel", (switch_time - time).round)
  end
  @time = switch_time
end

Then(/^node "(.*?)" should have (\d+) (\w+) transactions?$/) do |arg1, arg2, unit_name|
  @listtransactions = @nodes[arg1].unit_rpc(unit(unit_name), "listtransactions")
  begin
    expect(@listtransactions.size).to eq(arg2.to_i)
  rescue RSpec::Expectations::ExpectationNotMetError
    require 'pp'
    pp @listtransactions
    raise
  end
end

Then(/^the (\d+)\S+ transaction should be a send of "([^"]*?)" to "(.*?)"$/) do |arg1, arg2, arg3|
  begin
    tx = @listtransactions[arg1.to_i - 1]
    expect(tx["category"]).to eq("send")
    expect(tx["amount"]).to eq(-parse_number(arg2))
    expect(tx["address"]).to eq(@addresses[arg3])
  rescue RSpec::Expectations::ExpectationNotMetError
    require 'pp'
    pp @listtransactions
    raise
  end
end

Then(/^the (\d+)\S+ transaction should be a receive of "(.*?)" to "(.*?)"$/) do |arg1, arg2, arg3|
  tx = @listtransactions[arg1.to_i - 1]
  expect(tx["category"]).to eq("receive")
  expect(tx["amount"]).to eq(parse_number(arg2))
  expect(tx["address"]).to eq(@addresses[arg3])
end

Then(/^the transaction should be a send of "([^"]*?)" to "(.*?)"$/) do |arg1, arg2|
  step "the 1st transaction should be a send of \"#{arg1}\" to \"#{arg2}\""
end

Then(/^the transaction should be a receive of "(.*?)" to "(.*?)"$/) do |arg1, arg2|
  step "the 1st transaction should be a receive of \"#{arg1}\" to \"#{arg2}\""
end

Then(/^the transaction should be a receive of "([^"]*?)"$/) do |arg1|
  tx = @listtransactions[0]
  expect(tx["category"]).to eq("receive")
  expect(tx["amount"]).to eq(parse_number(arg1))
end

Then(/^the (\d+)\S+ transaction should be a send of "([^"]*?)"$/) do |arg1, arg2|
  tx = @listtransactions[arg1.to_i - 1]
  expect(tx["category"]).to eq("send")
  expect(tx["amount"]).to eq(-parse_number(arg2))
end

Then(/^the (\d+)st transaction should be the initial distribution of shares$/) do |arg1|
  tx = @listtransactions[arg1.to_i - 1]
  expect(tx["category"]).to eq("receive")
  expect(tx["amount"]).to eq(parse_number("10,000,000"))
end

Then(/^block "(.*?)" should contain transaction "(.*?)"$/) do |arg1, arg2|
  node = @nodes.values.first
  block = node.rpc("getblock", @blocks[arg1])
  expect(block["tx"]).to include(@tx[arg2])
end

Then(/^node "(.*?)" prints the last block$/) do |arg1|
  node = @nodes[arg1]
  require 'pp'
  pp node.top_block
end

Then(/^(\d+) seconds? pass(?:es|)$/) do |arg1|
  time_travel(arg1.to_i)
end

When(/^the error on node "(.*?)" should be "(.*?)"$/) do |arg1, arg2|
  node = @nodes[arg1]
  error = arg2

  wait_for do
    expect(node.info["errors"]).to eq(error)
  end
end

Given(/^node "(.*?)" sets (?:his|her) vote to:$/) do |arg1, string|
  @nodes[arg1].rpc("setvote", JSON.parse(string))
end

When(/^node "(.*?)" finds a block "(.*?)" on top of(?: block|) "(.*?)"$/) do |node, block, parent|
  @blocks[block] = @nodes[node].generate_stake(@blocks[parent])
end

Then(/^node "(.*?)" should have (\d+) connection$/) do |arg1, arg2|
  expect(@nodes[arg1].info["connections"]).to eq(arg2.to_i)
end

Given(/^node "(.*?)" grants (?:her|him)self "(.*?)" (\w+)$/) do |arg1, arg2, unit_name|
  address_name = "custodian_#{arg1}"
  step "node \"#{arg1}\" generates a #{unit_name} address \"#{address_name}\""
  step "node \"#{arg1}\" votes an amount of \"#{arg2}\" for custodian \"#{address_name}\""
  step "node \"#{arg1}\" finds blocks until custodian \"#{address_name}\" is elected"
end

Given(/^node "(.*?)" finds blocks until just received NSR are able to mint$/) do |arg1|
  node = @nodes[arg1]
  5.times do
    node.rpc("generatestake")
    time_travel(3 * 60)
  end
end

Given(/^node "(.*?)" finds blocks until voted park rate becomes effective$/) do |arg1|
  step "node \"#{arg1}\" finds 5 blocks received by all other nodes"
end

When(/^node "(.*?)" unparks the last park of node "(.*?)" with an amount of "(.*?)" (\w+)$/) do |arg1, arg2, arg3, arg4|
  unparking_node = @nodes[arg1]
  park_node = @nodes[arg2]
  amount = parse_number(arg3)
  unit = unit(arg4)
  parks = park_node.unit_rpc(unit, "listparked")
  last_park = parks.last
  hash = last_park["txid"]
  output = last_park["output"]
  unpark_address = last_park["unparkaddress"]
  unparking_node.unit_rpc(unit, "manualunpark", hash, output, unpark_address, amount)
end

Then(/^node "(.*?)" should stay at (\d+) transactions in memory pool$/) do |arg1, arg2|
  node = @nodes[arg1]
  count = arg2.to_i
  expect(node.rpc("getmininginfo")["pooledtx"]).to eq(count)
  sleep 2
  expect(node.rpc("getmininginfo")["pooledtx"]).to eq(count)
end

When(/^node "(.*?)" imports the private key "(.*?)" into the (\S+) wallet$/) do |arg1, arg2, arg3|
  node = @nodes[arg1]
  private_key = arg2
  unit_name = arg3
  node.unit_rpc(unit(unit_name), "importprivkey", private_key)
end
