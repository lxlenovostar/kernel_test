set pagination off
set logging file debug.log
set logging overwrite
set logging on
start
set $addr1 = pthread_mutex_lock
set $addr2 = pthread_mutex_unlock
b *$addr1
b *$addr2
while 1
    c
    if $pc != $addr1 && $pc != $addr2
        quit
    end
    bt
end
