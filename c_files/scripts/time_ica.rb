#!/usr/bin/env ruby

# Prints runtime info using various configurations of the ica_test program.
# This program takes a while to run.

run_strs = [
             "bin/ica_test -k -i jade  data/x1.csv",
             "bin/ica_test -k -c tanh  data/x1.csv",
             "bin/ica_test -k -c cube  data/x2.csv",
             "bin/ica_test -k -c gauss data/x3.csv",
             "bin/ica_test -k -i jade  data/x4.csv",
             "bin/ica_test -k -c tanh  data/x4.csv",
             "bin/ica_test -k -c cube  data/x5.csv",
             "bin/ica_test -k -c gauss data/x6.csv",
             "bin/ica_test -k -i jade  data/x7.csv",
             "bin/ica_test -k -c tanh  data/x7.csv",
             "bin/ica_test -k -c cube  data/x8.csv",
             "bin/ica_test -k -c gauss data/x9.csv",
           ]

CPU = 0
GPU = 1
times = [ [], [] ]

# Gather several samples of each runtime so that we can find the average and
# variance of each runtime.
0.upto(run_strs.size - 1) do |i|
  times[CPU][i] = []
  times[GPU][i] = []

  if i % 4 == 0
    case i / 4
      when 0
        puts "Gathering runtime info for generated sources..."
      when 1
        puts "Gathering runtime info for gaussian sources..."
      when 2
        puts "Gathering runtime info for EEG data..."
    end
  end

  case i % 4
    when 0
      puts "  testing JADE implementation..."
    when 1
      puts "  testing 'tanh' contrast rule..."
    when 2
      puts "  testing 'cube' contrast rule..."
    when 3
      puts "  testing 'gauss' contrast rule..."
  end

  valid = true
  20.times do
    output_str = `#{run_strs[i]}`

    cpu_iterations = output_str[ /CPU iterations\/sweeps: (.*)/, 1 ].to_i
    gpu_iterations = output_str[ /GPU iterations\/sweeps: (.*)/, 1 ].to_i
    times[CPU][i] << output_str[ /CPU execution time: (.*) seconds./, 1 ].to_f
    times[GPU][i] << output_str[ /GPU execution time: (.*) seconds./, 1 ].to_f

    if i == 5 or i == 6 or i == 7
      # Gaussian sources should take the maximum number of iterations.
      if cpu_iterations != 400 or gpu_iterations != 400
        valid = false
      end
    end

    if output_str =~ /NaN/
      puts "    rule resulted in NaN output!"
    end
  end

  if not valid
    puts "    rule did not take maximum number of iterations for Gaussian data!"
    puts "    Timing results are invalid for this rule!"
  end
end

# Find average times.
avg_times = [ [], [] ]
std_times = [ [], [] ]

0.upto(times[0].size - 1) do |i|
  avg = [ 0, 0 ]
  times[CPU][i].each do |t|
    avg[CPU] += t
  end
  times[GPU][i].each do |t|
    avg[GPU] += t
  end

  avg[CPU] /= times[0][0].size
  avg[GPU] /= times[0][0].size

  avg_times[CPU] << avg[CPU]
  avg_times[GPU] << avg[GPU]
end

0.upto(times[0].size - 1) do |i|
  std = [ 0, 0 ]
  times[CPU][i].each do |t|
    std[CPU] += (t - avg_times[CPU][i]) ** 2
  end
  times[GPU][i].each do |t|
    std[GPU] += (t - avg_times[GPU][i]) ** 2
  end

  std[CPU] /= times[0][0].size
  std[GPU] /= times[0][0].size

  std_times[CPU] << std[CPU]
  std_times[GPU] << std[GPU]
end

# Print data in a table that will look like:
#         |        CPU        |        CUDA       |    Speedup    |
#  rule   | Avg (s) | Std (s) | Avg (s) | Std (s) | CPU over CUDA |
#  -------+---------+---------+---------+---------+---------------+
#  JADE:  |   x.xx  |   x.xx  |   x.xx  |   x.xx  |     xx.xx     |
#  tanh:  |   x.xx  |   x.xx  |   x.xx  |   x.xx  |     xx.xx     |
#  cube:  |   x.xx  |   x.xx  |   x.xx  |   x.xx  |     xx.xx     |
#  gauss: |   x.xx  |   x.xx  |   x.xx  |   x.xx  |     xx.xx     |

str = <<EOS
Generated data:
         |        CPU        |        CUDA       |    Speedup    |
  rule   | Avg (s) | Std (s) | Avg (s) | Std (s) | CPU over CUDA |
  -------+---------+---------+---------+---------+---------------+
  JADE:  |   %0.2f  |   %0.2f  |   %0.2f  |   %0.2f  |     %0.2f      |
  tanh:  |   %0.2f  |   %0.2f  |   %0.2f  |   %0.2f  |     %0.2f      |
  cube:  |   %0.2f  |   %0.2f  |   %0.2f  |   %0.2f  |     %0.2f      |
  gauss: |   %0.2f  |   %0.2f  |   %0.2f  |   %0.2f  |     %0.2f      |

Gaussian data:
         |        CPU        |        CUDA       |    Speedup    |
  rule   | Avg (s) | Std (s) | Avg (s) | Std (s) | CPU over CUDA |
  -------+---------+---------+---------+---------+---------------+
  JADE:  |   %0.2f  |   %0.2f  |   %0.2f  |   %0.2f  |     %0.2f      |
  tanh:  |   %0.2f  |   %0.2f  |   %0.2f  |   %0.2f  |     %0.2f      |
  cube:  |   %0.2f  |   %0.2f  |   %0.2f  |   %0.2f  |     %0.2f      |
  gauss: |   %0.2f  |   %0.2f  |   %0.2f  |   %0.2f  |     %0.2f      |

EEG data:
         |        CPU        |        CUDA       |    Speedup    |
  rule   | Avg (s) | Std (s) | Avg (s) | Std (s) | CPU over CUDA |
  -------+---------+---------+---------+---------+---------------+
  JADE:  |   %0.2f  |   %0.2f  |   %0.2f  |   %0.2f  |     %0.2f      |
  tanh:  |   %0.2f  |   %0.2f  |   %0.2f  |   %0.2f  |     %0.2f      |
  cube:  |   %0.2f  |   %0.2f  |   %0.2f  |   %0.2f  |     %0.2f      |
  gauss: |   %0.2f  |   %0.2f  |   %0.2f  |   %0.2f  |     %0.2f      |
EOS

str = str % [
avg_times[CPU][0], std_times[CPU][0], avg_times[GPU][0], std_times[GPU][0], (avg_times[CPU][0] / avg_times[GPU][0]),
avg_times[CPU][1], std_times[CPU][1], avg_times[GPU][1], std_times[GPU][1], (avg_times[CPU][1] / avg_times[GPU][1]),
avg_times[CPU][2], std_times[CPU][2], avg_times[GPU][2], std_times[GPU][2], (avg_times[CPU][2] / avg_times[GPU][2]),
avg_times[CPU][3], std_times[CPU][3], avg_times[GPU][3], std_times[GPU][3], (avg_times[CPU][3] / avg_times[GPU][3]),

avg_times[CPU][4], std_times[CPU][4], avg_times[GPU][4], std_times[GPU][4], (avg_times[CPU][4] / avg_times[GPU][4]),
avg_times[CPU][5], std_times[CPU][5], avg_times[GPU][5], std_times[GPU][5], (avg_times[CPU][5] / avg_times[GPU][5]),
avg_times[CPU][6], std_times[CPU][6], avg_times[GPU][6], std_times[GPU][6], (avg_times[CPU][6] / avg_times[GPU][6]),
avg_times[CPU][7], std_times[CPU][7], avg_times[GPU][7], std_times[GPU][7], (avg_times[CPU][7] / avg_times[GPU][7]),

avg_times[CPU][8], std_times[CPU][8], avg_times[GPU][8], std_times[GPU][8], (avg_times[CPU][8] / avg_times[GPU][8]),
avg_times[CPU][9], std_times[CPU][9], avg_times[GPU][9], std_times[GPU][9], (avg_times[CPU][9] / avg_times[GPU][9]),
avg_times[CPU][10], std_times[CPU][10], avg_times[GPU][10], std_times[GPU][10], (avg_times[CPU][10] / avg_times[GPU][10]),
avg_times[CPU][11], std_times[CPU][11], avg_times[GPU][11], std_times[GPU][11], (avg_times[CPU][11] / avg_times[GPU][11]),
]

puts
puts str
