# Operation

## operation pseudocode
```
startup:

read eeprom to see if we have any data saved, 
meaning we have already paired to a remote and/or base

data should contain: 
- rf channel
- address (base + pipe)

if saved data is found, setup radio from saved data
else setup radio with random defaults & save

update display for address & channel

main active mode:
  
    check for keypress
        if keypress found, 
        handle keypress

    check & refresh display:
        if menu state changed
            redraw new menu
            set current menu state = new menu state

    check menu state for teach/learn
        if learn:
            do learn
        else if teach:
            wait for enter keypress
            do teach

function handle keypress:
  if menu buttons:
    check next menu item
    change new menu state to new menu item
  else if action buttons:
    do tx action    

function teach:
    setup pairing message
    scroll through all channels
    broadcasting pairing message
    checking for confirmation
    restore radio settings

function learn:
    setup radio addresses for learning
    watch for pairing message from remote
        if pairing message received
            acknowledge
            change radio settings
    
function change radio settings:
    setup radio settings (channel & address)
    save setttings to eeprom

```

## Display mocks:

#### Normal page:
```
a:0123abcdef
normal mode
remote
control
```

#### Main Menu page:
```
Main Menu:
Learn Remote
Teach Base
Exit
```
Arrow keys cycle through these entries, highlighting selected one. 
Hitting enter, takes you to selected menu.
Exit returns to normal display mode.

#### Menu, learn start:
```
Learn Mode:
Now press
sync on
remote

```
Change sleep timer to 30s for timeout.


#### Menu, learn finished:
```
Learn Mode:

Completed!

```
After 3s delay, go back to main menu.

#### Menu, teach base start:
```
Teach Base:
Start sync
on base then
press enter
```
Change sleep timer to 30s for timeout.

#### Menu, teach base completed:
```
Teach Base:

Completed!

```
(after 3s, go back to main menu)


https://github.com/olikraus/u8g2/wiki/u8x8reference#setinversefont