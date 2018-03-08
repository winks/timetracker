# TimeTracker

A simple GUI time tracker.

## Dependencies

  * Qt 5
  * QtDBus

## Building

```
qmake
make

```

## HowTo DBus

start tracking:

```
dbus-send --session --dest=org.f5n.timetracker --type=method_call --print-reply /TimeTracker org.f5n.timetracker.startTracking
```

stop tracking:

```
dbus-send --session --dest=org.f5n.timetracker --type=method_call --print-reply /TimeTracker org.f5n.timetracker.stopTracking
```


## License

BSD
