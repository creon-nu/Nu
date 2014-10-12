Feature: When some NuBits are parked, the change should be immediatly spendable like in normal transactions.
  Scenario: Some coins are parked and the change is spent
    Given a network with nodes "Shareholder", "Custodian" and "Recipient"
    When node "Custodian" generates a NuBit address "cust"
    And node "Shareholder" votes an amount of "1,000,000" for custodian "cust"
    And node "Shareholder" votes a park rate of "0.00000001" NuBits per Nubit parked during 4 blocks
    And node "Shareholder" finds blocks until custodian "cust" is elected
    And node "Shareholder" finds blocks until the NuBit park rate for 4 blocks is "0.00000001"
    And all nodes reach the same height
    Then the NuBit balance of node "Custodian" should reach "1,000,000"

    When node "Recipient" generates a NuBit address "recipient"
    And node "Custodian" parks "200,000" NuBits during 4 blocks
    And node "Custodian" sends "700,000" NuBits to "recipient"
    Then all nodes should have 2 transaction in memory pool
    When node "Shareholder" finds a block
    Then the NuBit balance of node "Recipient" should reach "700,000"
    And the NuBit balance of node "Custodian" should reach "99,999.98"
