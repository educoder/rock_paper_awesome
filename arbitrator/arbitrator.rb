require 'rubygems'
require 'blather/client'

setup 'arbitrator@badger.encorelab.org', 'arbitr@t0r'

when_ready { puts "Connected" }

# Auto approve subscription requests
subscription :request? do |s|
  write_to_stream s.approve!
end

# Echo back what was said
# message :chat?, :body do |m|
#   write_to_stream m.reply
# end

message :chat?, :body  => /gc/ do |m|
  write_to_stream m.reply
  b = m.body.reverse
  puts "Message #{b.inspect}"
  m.body = b
  write_to_stream m.reply
end

# The following will reconnect every time the connection is lost:
disconnected {client.connect}