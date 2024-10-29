import matplotlib.pyplot as plt

times = {
    'Base Floating Point': {'With Cache': 75, 'Without Cache': 2420},
    'Custom Floating Point': {'With Cache': 25, 'Without Cache': 804},
    'Fixed Point': {'With Cache': 1.8, 'Without Cache': 54},
}

# plot a histogram
fig, ax = plt.subplots()
bar_width = 0.35
bar_positions = range(len(times))
bar_positions_with_cache = [x - bar_width/2 for x in bar_positions]
bar_positions_without_cache = [x + bar_width/2 for x in bar_positions]

bars_with_cache = ax.bar(bar_positions_with_cache, [times[impl]['With Cache'] for impl in times], bar_width, label='With Cache')
bars_without_cache = ax.bar(bar_positions_without_cache, [times[impl]['Without Cache'] for impl in times], bar_width, label='Without Cache')

ax.set_xlabel('Implementation')
ax.set_ylabel('Time (s)')
ax.set_yscale('log')
ax.set_title('Execution time with and without cache')
ax.set_xticks(bar_positions)
ax.set_xticklabels(times.keys())
ax.legend()

plt.show()
