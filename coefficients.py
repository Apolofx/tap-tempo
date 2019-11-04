import numpy as np
arr = list()
for i in range(0, 256):
    arr.append(i)
delay = list()
delay = list(map(lambda var: int(var.strip(',')), input("Pega aca el array de delays:\n").split()))
for i in delay:
	i = int(i)
poly = np.polyfit(delay, arr, 2)
print("Pegar los coeficientes en los float del codigo de operacion")
print("a = "+str(poly[0])+", b = "+str(poly[1])+", c = "+str(poly[2]))
