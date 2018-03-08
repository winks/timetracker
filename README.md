# TimeTracker

A simple GUI time tracker. You can start and stop the timer.
Your progress will be saved to `~/timetracker.sqlite`.

It's better than Clicker Heroes.

## Dependencies

  * Qt 5
  * QtDBus
  * sqlite3

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
