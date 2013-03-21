#!/usr/bin/env ruby

run_strs = [
  "bin/src_test -e ../eeg_data/with_blink/normal/normal0.edf -o n0.csv",
  "bin/src_test -e ../eeg_data/with_blink/normal/normal1.edf -o n1.csv",
  "bin/src_test -e ../eeg_data/with_blink/normal/normal2.edf -o n2.csv",
  "bin/src_test -e ../eeg_data/with_blink/normal/normal3.edf -o n3.csv",
  "bin/src_test -e ../eeg_data/with_blink/normal/normal4.edf -o n4.csv",
  "bin/src_test -e ../eeg_data/with_blink/normal/normal5.edf -o n5.csv",
]

t_blink = 0.0
t_ica = 0.0
t_full = 0.0

run_strs.each do |run_str|
  output = `#{run_str}`

  t_blink += output[/blinks:\s+([0-9.]+)/,1].to_f
  t_ica   += output[/ICA:\s+([0-9.]+)/,1].to_f
  t_full  += output[/algorithm:\s+([0-9.]+)/,1].to_f
end

puts "Average time to detect blinks:   %1.4f seconds" % (t_blink / 6.0)
puts "Average time to perform ICA:     %1.4f seconds" % (t_ica / 6.0)
puts "Average time for full algorithm: %1.4f seconds" % (t_full / 6.0)
