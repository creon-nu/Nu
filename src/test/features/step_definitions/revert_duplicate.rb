When(/^node "(.*?)" sends a duplicate "(.*?)" of block "(.*?)"$/) do |node, duplicate, original|
  @blocks[duplicate] = @nodes[node].rpc("duplicateblock", @blocks[original])
end
