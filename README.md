# csse-2310
UQ's CSSE2310 was a hell of a course that had some "entertaining" projects within
it, these three projects offer a variety of topics they taught us.

# Bark
'Bark' is a command line based game where each player is dealt cards from the deck
and may play them to any slot of the board such that it is adjacent to an existing
card (which may wrap around the top/bottom and left/right of the board).

Scores are calculated by finding the longest string of cards in a player's suit
that are connected, with the game ending when all cards are played.

# 2310 Hub
'2310hub' is a CLI for multiple AI players (2310alice, 2310bub) to play a card
game not dissimilar to the classic Spades where each player must (should) play a
card of the same suit as that of the first player. In the case they do not have
one, they may play any card.

Scores are calculated according to if each player has played enough 'D' cards,
and how many cards they played with the highest score.

# 2310 Depot
'2310depot' is a CLI that interacts with neighbour instances to mimic transfers
between warehouses, at any point it accepts SIGHUP to display the wares within it,
and transfers wares to other instances according to STDIN arguments.
