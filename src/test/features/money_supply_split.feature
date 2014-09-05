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
    When node "B" generates a "NuBit" address "cust"
    And node "A" votes an amount of "1,000,000" for custodian "cust"
    And node "A" finds blocks until custodian "cust" is elected
    Then the "NuShare" supply should be between "1,000,000,000" and "1,000,100,000"
    And the "NuBit" supply should be "1,000,000"
