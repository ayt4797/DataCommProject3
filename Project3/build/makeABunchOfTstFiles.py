import shutil

for i in range (0,100):
    shutil.copyfile("tst.txt", "tst"+str(i)+".txt")
    shutil.copyfile("tst.txt", "../server/tst"+str(i)+".txt")


