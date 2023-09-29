require "pathname"

GAYA_EXE = "./build/src/gaya"

class String
  def run_test = system("#{GAYA_EXE} #{self} >/dev/null")
end

def test_error(test_file) = !test_file.run_test
def test_no_error(test_file) = test_file.run_test

successes = 0
errors = 0

Pathname.new("tests").children.collect(&:to_s).each do |test|
  next if not test.end_with?(".gaya")

  line = File.read(test).lines.first
  ok =  line =~ /Expect error/ ? test_error(test) : test_no_error(test)

  if ok 
    puts "\x1b[32m✅ #{test}\x1b[m"
    successes += 1
  else
    puts "\x1b[31m❌ #{test}\x1b[m"
    errors += 1
  end
end

puts "Summary: #{successes} successes, #{errors} failures"

exit errors
