#!/usr/bin/env ruby

################################################################################
# Gather results for real, normal EEG data.
################################################################################
run_strs = [
  "bin/bd_test -e ../eeg_data/with_blink/normal/normal0.edf -b 3135",
  "bin/bd_test -e ../eeg_data/with_blink/normal/normal1.edf -b 2340",
  "bin/bd_test -e ../eeg_data/with_blink/normal/normal2.edf -b 493 -b 3870",
  "bin/bd_test -e ../eeg_data/with_blink/normal/normal3.edf -b 2247",
  "bin/bd_test -e ../eeg_data/with_blink/normal/normal4.edf -b 182 -b 3319",
  "bin/bd_test -e ../eeg_data/with_blink/normal/normal5.edf -b 1802",
  "bin/bd_test -e ../eeg_data/no_blink/normal/normal0.edf",
  "bin/bd_test -e ../eeg_data/no_blink/normal/normal1.edf",
  "bin/bd_test -e ../eeg_data/no_blink/normal/normal2.edf",
  "bin/bd_test -e ../eeg_data/no_blink/normal/normal3.edf",
  "bin/bd_test -e ../eeg_data/no_blink/normal/normal4.edf",
  "bin/bd_test -e ../eeg_data/no_blink/normal/normal5.edf",
  "bin/bd_test -e ../eeg_data/no_blink/triphasic/triphasic0.edf",
  "bin/bd_test -e ../eeg_data/no_blink/triphasic/triphasic1.edf",
  "bin/bd_test -e ../eeg_data/no_blink/triphasic/triphasic2.edf",
  "bin/bd_test -e ../eeg_data/no_blink/triphasic/triphasic3.edf",
  "bin/bd_test -e ../eeg_data/no_blink/triphasic/triphasic4.edf",
  "bin/bd_test -e ../eeg_data/no_blink/triphasic/triphasic5.edf",
]

found = 0
exp_det = 0
false_det = 0
missed_det = 0

0.upto(run_strs.size - 1) do |i|
  output = `#{run_strs[i]}`

  found      += output[/Expected \d+ blink\(s\), found (\d+) blink/,1].to_i
  exp_det    += output[/Expected (\d+) blink/,1].to_i
  false_det  += output[/False detection count: (.*)/,1].to_i
  missed_det += output[/Missed detection count: (.*)/,1].to_i
end

puts
puts "Real EEG data:"
puts "  Total expected and found: #{exp_det}, #{found}"
puts "  Total false detections:   #{false_det}"
puts "  Total missed detections:  #{missed_det}"

found = 0
exp_det = 0
false_det = 0
missed_det = 0

################################################################################
# Gather results for simulated, normal EEG data.
################################################################################

blinks = [
  "-b 3135",          # Blinks in normal0
  "-b 2340",          # Blinks in normal1
  "-b 493 -b 3870",   # Blinks in normal2
  "-b 2247",          # Blinks in normal3
  "-b 182 -b 3319",   # Blinks in normal4
  "-b 1802"           # Blinks in normal5
]

Dir.glob("../eeg_data/simulated/sim_n*_n*.edf").each do |file|
  run_str = "bin/bd_test -e #{file}"

  # Append the expected location of blinks to the run string.
  0.upto(5) do |i|
    if run_str =~ Regexp.new( "sim_n#{i}" )
      run_str = run_str + " " + blinks[i]
    end
  end

  output = `#{run_str}`

  found      += output[/Expected \d+ blink\(s\), found (\d+) blink/,1].to_i
  exp_det    += output[/Expected (\d+) blink/,1].to_i
  false_det  += output[/False detection count: (.*)/,1].to_i
  missed_det += output[/Missed detection count: (.*)/,1].to_i
end

puts
puts "Simulated normal EEG data:"
puts "  Total expected and found: #{exp_det}, #{found}"
puts "  Total false detections:   #{false_det}"
puts "  Total missed detections:  #{missed_det}"

################################################################################
# Gather results for simulated, triphasic EEG data.
################################################################################

found = 0
exp_det = 0
false_det = 0
missed_det = 0

Dir.glob("../eeg_data/simulated/sim_n*_t*.edf").each do |file|
  run_str = "bin/bd_test -e #{file}"

  # Append the expected location of blinks to the run string.
  0.upto(5) do |i|
    if run_str =~ Regexp.new( "sim_n#{i}" )
      run_str = run_str + " " + blinks[i]
    end
  end

  output = `#{run_str}`

  found      += output[/Expected \d+ blink\(s\), found (\d+) blink/,1].to_i
  exp_det    += output[/Expected (\d+) blink/,1].to_i
  false_det  += output[/False detection count: (.*)/,1].to_i
  missed_det += output[/Missed detection count: (.*)/,1].to_i
end

puts
puts "Simulated triphasic EEG data:"
puts "  Total expected and found: #{exp_det}, #{found}"
puts "  Total false detections:   #{false_det}"
puts "  Total missed detections:  #{missed_det}"
