Feature: Split money supply
  The money supply is tracked independently for each unit

  Scenario: Initial money supply of the network
    Given a network with node "A" able to mint
    Then the "NuShare" supply should be between "1,000,000,000" and "1,000,100,000"
    And the "NuBit" supply should be "0"

  Scenario: Money supply after a block is found
    Given a network with nodes "A" and "B" able to mint
    Then "NuShare" money supply on node "B" should increase by "40" when node "A" finds a block
    And "NuBit" money supply on node "B" should increase by "0" when node "A" finds a block

  Scenario: Money supply after a custodian is elected
    Given a network with nodes "A" and "B" able to mint
    When node "B" generates a NuBit address "cust"
    And node "A" votes an amount of "1,000,000" for custodian "cust"
    And node "A" finds blocks until custodian "cust" is elected
    Then the "NuShare" supply should be between "1,000,000,000" and "1,000,100,000"
    And the "NuBit" supply should be "1,000,000"

  Scenario: NuBit money supply after parking and unparking
    Given a network with nodes "Alice" and "Bob" able to mint
    Then the total parked NuBits should reach "0"

    When node "Bob" generates a NuBit address "cust"
    And node "Alice" votes an amount of "1,000,000" for custodian "cust"
    And node "Alice" votes a park rate of "0.00000001" NuBits per Nubit parked during 4 blocks
    And node "Alice" finds blocks until custodian "cust" is elected
    And node "Bob" reaches a balance of "1,000,000" NuBits
    And the current NuShare supply on "Bob" is recorded
    And node "Bob" parks "50,000" NuBits for 4 blocks

    When node "Bob" finds a block
    Then the "NuBit" supply for "Bob" should be "999,999.99"
    And the "NuShare" supply for "Bob" should have increased by "40"
    And the amount minted on the last block on "Bob" should be "40"

    When node "Bob" finds 4 blocks
    And node "Bob" unparks
    When node "Bob" finds a block
    Then the "NuBit" supply for "Bob" should be "999,999.9905"
