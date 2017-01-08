import maya.cmds as cmds

cmds.select(ado = True)
nodes = cmds.ls("pCube*", sl = True)

count = 0

for node in nodes:
    cmds.select(node)
    cmds.file("E:/mattt/Documents/MPC/objFiles/Voronoi2/cubeFrag" + str(count)+".obj", op = "groups=0;ptgroups=0;materials=0;smoothing=0;normals=1", type = "OBJexport", es = True)
    count += 1

print nodes