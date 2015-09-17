print(...)

local tb = {
    a = 1,
    b = 2,
}

local a = 1
a = 2
a = 6

local fn = function()
    print(hello)
end

function calc_now2time(day, hour, min, sec)
    local now = os.time()
    return now
end

a = 7
a = calc_now2time()
print("a",a)
