platform := "emery"

default: build

build:
    pebble build

clean:
    pebble clean

# Regenerate compile_commands.json for clangd
db:
    pebble clean
    bear -- pebble build

run: build
    pebble install --emulator {{platform}}

logs:
    pebble logs --emulator {{platform}}

# Install to a real watch over the dev connection
deploy: build
    pebble install --phone $PEBBLE_PHONE

screenshot:
    pebble screenshot --emulator {{platform}}
