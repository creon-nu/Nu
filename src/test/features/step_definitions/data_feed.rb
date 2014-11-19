
Before do
  @data_feeds = {}
end

class DataFeedContainer
  def initialize(options = {})
    default_options = {
      image: "nubit/data_feed",
    }

    options = default_options.merge(options)

    bash_cmds = []

    bash_cmds += ["ruby /data_feed.rb -o 0.0.0.0"]

    command = [
      "stdbuf", "-oL", "-eL",
      '/bin/bash', '-c',
      bash_cmds.join("; "),
    ]

    create_options = {
      'Image' => options[:image],
      'WorkingDir' => '/',
      'Tty' => true,
      'Cmd' => command,
      'ExposedPorts' => {
        "4567/tcp" => {},
      },
    }
    @container = Docker::Container.create(create_options)

    start_options = {
      'PortBindings' => {
        "4567/tcp" => ['127.0.0.1'],
      },
    }

    sleep 0.1
    @container.start(start_options)

    @json = @container.json
    @name = @json["Name"]

    retries = 0
    begin
      ports = @container.json["NetworkSettings"]["Ports"]
      if ports.nil?
        raise "Unable to get port. Usualy this means the daemon process failed to start. Container was #{@name}"
      end
    rescue
      if retries >= 3
        raise
      else
        sleep 0.1
        retries += 1
        retry
      end
    end

    @port = ports["4567/tcp"].first["HostPort"].to_i
    @host_ip = @container.json["NetworkSettings"]["Gateway"]
  end

  def url(path = '')
    "http://#@host_ip:#@port/#{path}"
  end

  def wait_for_boot
    require 'httparty'
    Timeout.timeout(10) do
      begin
        response = HTTParty.get(url)
        raise unless response.code == 200
      rescue Errno::ECONNREFUSED, RuntimeError
        sleep 0.1
        retry
      end
    end
  end

  def stop
    return unless @container

    @container.delete(force: true)
    @container = nil
  end

  def set(value)
    response = HTTParty.post(url('vote.json'), body: value)
    raise "Set vote failed" unless response.code == 200
    response = HTTParty.get(url('vote.json'))
    raise "Unable to retreive set vote" unless response.code == 200
    raise "Set vote mismatch" unless response.body == value
  end

  def set_status(value)
    response = HTTParty.post(url('status'), body: value)
    raise "Set status failed" unless response.code == 200
  end
end

Given(/^a data feed "(.*?)"$/) do |arg1|
  @data_feeds[arg1] = data_feed = DataFeedContainer.new
  data_feed.wait_for_boot
end

After do
  @data_feeds.values.each(&:stop)
end

Given(/^the data feed "(.*?)" returns:$/) do |arg1, string|
  @data_feeds[arg1].set(string)
end

Given(/^the data feed "(.*?)" returns a status (\d+) with the body:$/) do |arg1, arg2, string|
  @data_feeds[arg1].set(string)
  @data_feeds[arg1].set_status(arg2)
end

Given(/^the data feed "(.*?)" returns a valid vote of (\d+) bytes$/) do |arg1, arg2|
  data_feed = @data_feeds[arg1]
  size = arg2.to_i

  vote = %q(
      {
         "custodians":[
            {"address":"bPwdoprYd3SRHqUCG5vCcEY68g8UfGC1d9", "amount":100.00000000},
            {"address":"bxmgMJVaniUDbtiMVC7g5RuSy46LTVCLBT", "amount":5.50000000}
         ],
         "parkrates":[
            {
               "unit":"B",
               "rates":[
                  {"blocks":8192, "rate":0.00030000},
                  {"blocks":16384, "rate":0.00060000},
                  {"blocks":32768, "rate":0.00130000}
               ]
            }
         ],
         "motions":[
            "8151325dcdbae9e0ff95f9f9658432dbedfdb209",
            "3f786850e387550fdab836ed7e6dc881de23001b"
         ]
      }
  )
  vote += " " * (size - vote.size)
  data_feed.set(vote)
end

When(/^node "(.*?)" sets her data feed to the URL of "(.*?)"$/) do |arg1, arg2|
  @nodes[arg1].rpc("setdatafeed", @data_feeds[arg2].url("vote.json"))
end

Then(/^the vote of node "(.*?)" (?:should be|is|should become):$/) do |arg1, string|
  begin
    wait_for do
      expect(JSON.pretty_generate(@nodes[arg1].rpc("getvote"))).to eq(JSON.pretty_generate(JSON.parse(string)))
    end
  rescue
    errors = @nodes[arg1].info["errors"]
    puts errors unless errors.blank?
    raise
  end
end

Then(/^node "(.*?)" should use the data feed "(.*?)"$/) do |arg1, arg2|
  expect(@nodes[arg1].rpc("getdatafeed")).to eq(@data_feeds[arg2].url("vote.json"))
end

Given(/^data feed "(.*?)" shuts down$/) do |arg1|
  @data_feeds[arg1].stop
end
