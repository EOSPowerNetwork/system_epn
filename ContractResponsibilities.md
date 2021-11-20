# Contract Responsibilities

_Names of contracts and what they are used for_

## tokens.epn

- All regular token functions
- Manages the distribution event
- Swaps between tokens

## proxy.epn

- Proxies register
- Allows participation in power contest
- Votes for EOS BPs according to the winners of the contest

## system.epn

- Operators register
- Sets the permissions according to the top elected BPs
- Users create all payment contracts

## exec.epn

- Executes transfers according to the stored contracts

## revenue.epn

- Collection of all revenue generated by the EPN