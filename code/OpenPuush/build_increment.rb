unless File.exists? 'build_num'
  open('build_num', 'w') { |f| f.print 0 }
end

open('build_num', 'r+') do |f|
  c = f.read.to_i + 1
  f.seek 0
  f.print c
  print c
end
