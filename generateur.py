#!/usr/bin/python3

def grid(n=100):
    for x in range(-n//2, n//2):
        for y in range(-n//2, n//2):
            print('P', x*100, y*100, -x**2 - y**2)


if __name__ == "__main__":
    grid(200)
