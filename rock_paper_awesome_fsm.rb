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
      on :connect, :to => :CONNECTING, 
        :action => proc{|rpa| output("~") }
      on :connected, :to => :ONLINE,
        :if => proc{|input| input == "@"}
    end

    state :CONNECTING do
      on :connected, :to => :ONLINE,
        :if => proc{|input| input == "@"}
    end

    state :ONLINE do
      on :remote_ready, :to => :WAITING_FOR_YOUR_READY,
        :if => proc{|input| input == "N"}
      on :ready, :to => :WAITING_FOR_THEIR_READY, 
        :action => proc{|rpa| output("n") }
    end

    state :WAITING_FOR_THEIR_READY do
      on :remote_ready, :to => :READY_TO_PLAY,
        :if => proc{|input| input == "N"}
    end

    state :WAITING_FOR_YOUR_READY do
      on :ready, :to => :READY_TO_PLAY, 
        :action => proc{|rpa| output("n") }
    end

    state :READY_TO_PLAY do
      on :you_choose, :to => :WAITING_FOR_THEIR_CHOICE, 
        :action => proc{|rpa| output("r") or output("p") or output("s")}
      on :they_choose, :to => :WAITING_FOR_YOUR_CHOICE,
        :if => proc{|input| input == "R" or input == "P" or input == "S"}
    end

    state :WAITING_FOR_THEIR_CHOICE do
      on :they_choose, :to => :WAITING_FOR_RESULT,
        :if => proc{|input| input == "R" or input == "P" or input == "S"}
    end

    state :WAITING_FOR_YOUR_CHOICE do
      on :you_choose, :to => :WAITING_FOR_RESULT, 
        :action => proc{|rpa| output("r") or output("p") or output("s")}
    end

    state :WAITING_FOR_RESULT do
      on :you_win, :to => :ONLINE, 
        :if => proc{|input| input == "+"},
        :action => proc{|rpa| rpa.win_count += 1}
      on :you_lose, :to => :ONLINE, 
        :if => proc{|input| input == "-"}, 
        :action => proc{|rpa| rpa.lose_count += 1}
      on :tie, :to => :ONLINE, 
        :if => proc{|input| input == "="}
    end
  end
end

if __FILE__ == $0
  require 'golem/visualizer'
  rpa = RockPaperAwesome.new
  viz = Golem::Visualizer.new(rpa.statemachines[:arduino])
  viz.visualize("png", "rock_paper_awesome_fsm.png")
end
