Feature: Sending liquidity info with an identifier

  Scenario:
    Given a network with nodes "Alice" and "Custodian" able to mint
    When node "Custodian" generates a NuBit address "cust"
    And node "Alice" votes an amount of "1,000,000" for custodian "cust"
    And node "Alice" finds blocks until custodian "cust" is elected
    And all nodes reach the same height
    And node "Custodian" sends a liquidity of "1000" buy and "2000" sell on unit "B" from address "cust" with identifier "A"
    Then node "Alice" should reach a total liquidity info of "1000" buy and "2000" sell on unit "B"
    And 1 second passes
    And node "Custodian" sends a liquidity of "10" buy and "5" sell on unit "B" from address "cust" with identifier "B"
    Then node "Alice" should reach a total liquidity info of "1010" buy and "2005" sell on unit "B"
