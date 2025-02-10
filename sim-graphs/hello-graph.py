import pandas as pd
import matplotlib.pyplot as plt

# A dictionary which represents data
# 900 is a incorrect value
student_dict = {'name': ['Lupe', 'Priyanka', 'Jesse', 'Jun', 'Thao'],
                'age': [19, 20, 22, 25, 34],
                'grade': [91, 98, 78, 900, 78],
                }

# creating a data frame
df = pd.DataFrame(student_dict)
print(df)

# clean data at specific row,col
df.loc[3, 'grade'] = 90

# plot grades and student names
bar_colors = ['tab:red', 'tab:blue', 'tab:purple', 'tab:orange', 'tab:pink']
ax = df.plot(kind='bar', x='name', y='grade', figsize=(
    4, 4), legend=False, color=bar_colors)

# set title
ax.set_xlabel('Student')
ax.set_ylabel('Grade')

# plot the graph, and resize to fit in window
plt.subplots_adjust(bottom=.3, left=.15)
plt.show()
