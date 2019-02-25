import string
import random

#Function for creating 10 random lowercase characters
def randomchargen(size = 10, char_set=string.ascii_lowercase):
    return ''.join(random.choice(char_set) for _ in range(size))

#Create file 1 w/ 10 random chars
file1 = open("python1.txt", "w+")
randomchar1 = randomchargen()
print randomchar1
file1.write(randomchar1)
file1.write("\n")
file1.close()

#Create file 2
file2 = open("python2.txt", "w+")
randomchar2 = randomchargen()
print randomchar2
file2.write(randomchar2)
file2.write("\n")
file2.close()

#Create file 3
file3 = open("python3.txt", "w+")
randomchar3 = randomchargen()
print randomchar3
file3.write(randomchar3)
file3.write("\n")
file3.close()

#get 2 random ints between 1-42
rand1 = random.randint(1, 42)
rand2 = random.randint(1, 42)

#print two ints and product
print rand1
print rand2
rand3 = rand1 * rand2
print rand3
