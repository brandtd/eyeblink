#!/usr/bin/env ruby

File.open( 'wavelets_coeffs.h' ) do |f|

  while not f.eof

    line = f.readline
    if line !~ /^static double db/ and 
       line !~ /^static double sym/ and
       line !~ /^static double coif/ and
       line !~ /^double bior/ and
       line !~ /^double dmey/
      next
    end

    filename = line[ /double (.*?)\[/, 1 ] + ".m"

    File.open( filename, 'w' ) do |wf|
      wf.puts( "% Coefficient values taken from:" )
      wf.puts( "%   http://wavelets.pybytes.com/" )
      wf.puts( "" )

      lo_d = f.readline.gsub(/\s|\{|\}/,'').gsub(/,$/,'').split(',')
      hi_d = f.readline.gsub(/\s|\{|\}/,'').gsub(/,$/,'').split(',')
      lo_r = f.readline.gsub(/\s|\{|\}/,'').gsub(/,$/,'').split(',')
      hi_r = f.readline.gsub(/\s|\{|\}/,'').gsub(/,$/,'').split(',')

      wf.puts( "Lo_D = [ " )
      lo_d.each do |c|
        if c =~ /^-/
          wf.puts(  "         " + c + ", ..." )
        else
          wf.puts( "          " + c + ", ..." )
        end
      end
      wf.puts( "       ];" )

      wf.puts( "Hi_D = [ " )
      hi_d.each do |c|
        if c =~ /^-/
          wf.puts(  "         " + c + ", ..." )
        else
          wf.puts( "          " + c + ", ..." )
        end
      end
      wf.puts( "       ];" )

      wf.puts( "Lo_R = [ " )
      lo_r.each do |c|
        if c =~ /^-/
          wf.puts(  "         " + c + ", ..." )
        else
          wf.puts( "          " + c + ", ..." )
        end
      end
      wf.puts( "       ];" )

      wf.puts( "Hi_R = [ " )
      hi_r.each do |c|
        if c =~ /^-/
          wf.puts(  "         " + c + ", ..." )
        else
          wf.puts( "          " + c + ", ..." )
        end
      end
      wf.puts( "       ];" )
    end

  end
end
