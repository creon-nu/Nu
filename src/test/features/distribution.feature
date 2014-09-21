Feature: Distribution to shareholders
  Scenario: Distribution is sent to shareholders
    Given a network with nodes "Alice" able to mint
    And a node "Bob" with an empty wallet
    And a node "Charly" with an empty wallet

    When node "Bob" generates a NuShare address "bob1"
    And node "Bob" generates a NuShare address "bob2"
    And node "Charly" generates a NuShare address "charly"
    And node "Alice" sends "10,000" NuShares to "charly"
    And node "Alice" sends "20,000" NuShares to "bob1"
    And node "Alice" sends "15,456.3214" NuShares to "bob1"
    And node "Alice" finds 2 blocks
    And node "Alice" distributes "720,000" Peercoins

    Then the distribution should send "7.2" Peercoins to "charly", adjusted by the real NuShares supply
    And the distribution should send "25.528551" Peercoins to "bob1", adjusted by the real NuShares supply
    And the distribution should not send anything to "bob2"

  Scenario: Distribution just above and below the minimum payout
    Given a network with nodes "Alice" able to mint
    And a node "Bob" with an empty wallet

    When node "Bob" generates a NuShare address "above"
    And node "Bob" generates a NuShare address "below"
    And node "Alice" sends "13.89" NuShares to "above"
    And node "Alice" sends "13.88" NuShares to "below"
    And node "Alice" finds 2 blocks
    And node "Alice" distributes "720,000" Peercoins

    Then the distribution should send "0.01" Peercoins to "above", adjusted by the real NuShares supply
    And the distribution should not send anything to "below"

