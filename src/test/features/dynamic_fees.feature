Feature: Shareholders can vote for the minimum transaction fee of each unit
  Scenario: NSR fee changed by vote
    Given a network with nodes "Alice" and "Bob" able to mint
    And a node "Charlie" with an empty wallet
    And the network has switched to dynamic fees
    When node "Alice" votes for an NSR fee of 0.2
    And node "Alice" finds enough blocks for a fee vote to pass
    And node "Alice" finds enough blocks for the voted fee to be effective
    Then the NSR fee should be 0.2
    When all nodes reach the same height
    And node "Charlie" generates an NSR address "charlie"
    And node "Bob" sends "3,000" NSR to "charlie" in transaction "tx"
    Then transaction "tx" on node "Charlie" should have a fee of 0.2
    And all nodes should reach 1 transaction in memory pool
    When node "Alice" finds a block
    Then node "Charlie" should have a balance of "3,000" NSR

  Scenario: Fee vote before protocol switch
    Given a network with nodes "Alice" and "Bob" able to mint
    And a node "0.5" running version "0.5.2"
    When node "Alice" votes for an NSR fee of 0.2
    And node "Alice" finds enough blocks for a fee vote to pass
    And node "Alice" finds enough blocks for the voted fee to be effective
    Then the NSR fee should be 1
    And all nodes should be at the same height

  Scenario: NBT fee changed by vote
    Given a network with nodes "Alice" and "Bob" able to mint
    And a node "Charlie" with an empty wallet
    And node "Bob" generates a NuBit address "cust"
    And node "Alice" votes an amount of "1,000,000" for custodian "cust"
    And node "Alice" finds blocks until custodian "cust" is elected
    And all nodes reach the same height
    And the network has switched to dynamic fees
    When node "Alice" votes for an NBT fee of 0.05
    And node "Alice" finds enough blocks for a fee vote to pass
    And node "Alice" finds enough blocks for the voted fee to be effective
    Then the NBT fee should be 0.05
    When all nodes reach the same height
    And node "Charlie" generates an NBT address "charlie"
    And node "Bob" sends "1" NBT to "charlie" in transaction "tx"
    Then transaction "tx" on node "Charlie" should have a fee of 0.05
    And all nodes should reach 1 transaction in memory pool
    When node "Alice" finds a block
    Then node "Charlie" should have a balance of "1" NBT
    And node "Bob" should have a balance of "999,998.95" NBT

