require 'rubygems'
require 'golem'

class RockPaperAwesome
  include Golem
  
  attr_reader :your_weapon, :their_weapon

  def initialize
  end

  def set_your_weapon(weapon)
    @your_weapon = weapon
  end

  def set_their_weapon(weapon)
    @their_weapon = weapon
  end

  def you_won
    puts "YOU WON!"
  end

  def you_lost
    puts "YOU_LOST!"
  end

  def output(char)
    # TODO: write to serial
    puts char
  end


  define_statemachine :arduino do
    initial_state :OFFLINE
    #on_all_transitions :write_in_journal

    state :OFFLINE do
      on :connecting, :to => :CONNECTING
      on :connected, :to => :ONLINE
    end

    state :CONNECTING do
      on :connected, :to => :ONLINE
    end

    state :ONLINE do
      on :remote_ready, :to => :WAITING_FOR_YOUR_READY
      on :ready, :to => :WAITING_FOR_THEIR_READY
    end

    state :WAITING_FOR_THEIR_READY do
      on :remote_ready, :to => :READY_TO_PLAY
    end

    state :WAITING_FOR_YOUR_READY do
      on :ready, :to => :READY_TO_PLAY
    end

    state :READY_TO_PLAY do
      on :you_choose, :to => :WAITING_FOR_THEIR_CHOICE
      on :they_choose, :to => :WAITING_FOR_YOUR_CHOICE
    end

    state :WAITING_FOR_THEIR_CHOICE do
      on :they_choose, :to => :WAITING_FOR_RESULT
    end

    state :WAITING_FOR_YOUR_CHOICE do
      on :you_choose, :to => :WAITING_FOR_RESULT
    end

    state :WAITING_FOR_RESULT do
      on :you_win, :to => :ONLINE
      on :you_lose, :to => :ONLINE
      on :tie, :to => :ONLINE
    end
  end
end

if __FILE__ == $0
  require 'golem/visualizer'
  rpa = RockPaperAwesome.new
  viz = Golem::Visualizer.new(rpa.statemachines[:arduino])
  viz.visualize("png", "awesome_fsm.png")
end
