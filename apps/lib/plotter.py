import sys
import matplotlib.pyplot as plt

# coords should be (row,col)
def plot_2D_rmaj(coords, annotate=False):
    # plot origin
    plt.scatter(coords[0][1], coords[0][0], zorder=10, color='green', marker='X', s=85)
    plt.plot(coords[0][1], coords[0][0], zorder=0, linewidth=4, color='grey')
    # plot the rest
    plt.scatter([x for _,x in coords[1:]], [y for y,_ in coords[1:]], zorder=10, color='blue')
    plt.plot([x for _,x in coords], [y for y,_ in coords], zorder=0, linewidth=4, color='grey')
    if annotate:
        for i in range(len(coords)):
            plt.annotate(str(i), (coords[i][0], coords[i][1]), zorder=20)
    plt.gca().invert_yaxis()
    plt.gca().xaxis.tick_top()
    plt.show()


f = sys.argv[1]
fd = open(f, 'r')
coords = []
for line in fd:
    r,c = line.split(',')
    coords.append([int(r), int(c)])

plot_2D_rmaj(coords)
