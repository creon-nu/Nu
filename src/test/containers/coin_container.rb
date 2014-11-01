require 'docker'
require 'httparty'

class CoinContainer
  def initialize(options = {})
    default_options = {
      image: "nubit/base",
      bind_code: true,
      shutdown_at_exit: true,
      delete_at_exit: false,
      remove_addr_after_shutdown: true,
      remove_wallet_after_shutdown: false,
      remove_wallet_before_startup: false,
    }

    options = default_options.merge(options)

    links = options[:links]
    case links
    when Hash
      links = links.map { |link_name, alias_name| [link_name, alias_name] }
    when Array
      links = links.map do |n|
        name = case n
               when String then n
               when CoinContainer then n.name
               else raise "Unknown link: #{n.inspect}"
               end
        [name, name.sub(/^\//, '')]
      end
    when nil
      links = []
    else
      raise "Invalid links"
    end
    name = options[:name]

    connects = links.map do |linked_name, alias_name|
      upname = alias_name.upcase
      "-addnode=$#{upname}_PORT_7895_TCP_ADDR:$#{upname}_PORT_7895_TCP_PORT"
    end

    default_args = {
      testnet: true,
      printtoconsole: true,
      rpcuser: 'bob',
      rpcpassword: 'bar',
      rpcallowip: '*.*.*.*',
      logtimestamps: true,
      keypool: 1,
      stakegen: false,
      unpark: false,
    }

    args = default_args.merge(options[:args] || {})

    cmd_args = args.map do |key, value|
      case value
      when true
        "-#{key}"
      when false
        "-#{key}=0"
      when Numeric
        "-#{key}=#{value}"
      else
        "-#{key}=\"#{value}\""
      end
    end
    cmd_args += connects

    bash_cmds = []

    if options[:show_environment]
      bash_cmds += ["echo Environment:", "env"]
    end

    if options[:remove_wallet_before_startup]
      bash_cmds += ["rm -f ~/.nu/testnet/wallet*.dat"]
    end

    bash_cmds += ["./nud " + cmd_args.join(" ")]

    if options[:remove_addr_after_shutdown]
      bash_cmds += ["rm ~/.nu/testnet/addr.dat"]
    end

    if options[:remove_wallet_after_shutdown]
      bash_cmds += ["rm ~/.nu/testnet/wallet*.dat"]
    end

    command = [
      "stdbuf", "-oL", "-eL",
      '/bin/bash', '-c',
      bash_cmds.join("; "),
    ]

    create_options = {
      'Image' => options[:image],
      'WorkingDir' => '/code',
      'Tty' => true,
      'Cmd' => command,
      'ExposedPorts' => {
        "7895/tcp" => {},
        "15001/tcp" => {},
        "15002/tcp" => {},
      },
      'name' => name,
    }
    node_container = Docker::Container.create(create_options)

    if options[:shutdown_at_exit]
      at_exit do
        shutdown rescue nil
      end
    end
    if options[:delete_at_exit]
      at_exit do
        container.delete(force: true)
      end
    end

    start_options = {
      'PortBindings' => {
        "15001/tcp" => ['127.0.0.1'],
        "15002/tcp" => ['127.0.0.1'],
        "7895/tcp" => ['127.0.0.1'],
      },
      'Links' => links.map { |link_name, alias_name| "#{link_name}:#{alias_name}" },
    }

    if options[:bind_code]
      start_options['Binds'] = ["#{File.expand_path('../../..', __FILE__)}:/code"]
    end

    sleep 0.1
    node_container.start(start_options)

    @container = node_container
    @json = @container.json
    @name = @json["Name"]

    retries = 0
    begin
      ports = node_container.json["NetworkSettings"]["Ports"]
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

    @rpc_ports = {
      'S' => ports["15001/tcp"].first["HostPort"].to_i,
      'B' => ports["15002/tcp"].first["HostPort"].to_i,
    }
    @rpc_port = @rpc_ports['S']
    @port= ports["7895/tcp"].first["HostPort"].to_i
  end

  attr_reader :rpc_port, :port, :container, :name

  def json
    container.json
  end

  def id
    @json["Id"]
  end

  def rpc(method, *params)
    unit_rpc('S', method, *params)
  end

  def unit_rpc(unit, method, *params)
    data = {
      method: method,
      params: params,
      id: 'jsonrpc',
    }
    rpc_port = @rpc_ports.fetch(unit)
    url = "http://localhost:#{rpc_port}/"
    auth = {
      username: "bob",
      password: "bar",
    }
    response = HTTParty.post url, body: data.to_json, headers: { 'Content-Type' => 'application/json' }, basic_auth: auth
    result = JSON.parse(response.body)
    raise result.inspect if result["error"]
    result["result"]
  end

  def wait_for_boot
    begin
      rpc("getinfo")
    rescue Errno::ECONNREFUSED, Errno::ECONNRESET, EOFError, Errno::EPIPE
      sleep 0.1
      retry
    end
  end

  def commit(repo)
    image = container.commit
    image.tag 'repo' => repo
  end

  def shutdown
    rpc("shutdown")
  end

  def wait_for_shutdown
    container.wait
  end

  def block_count
    rpc("getblockcount").to_i
  end

  def generate_stake(parent = nil)
    rpc("generatestake", *[parent].compact)
  end

  def top_hash
    rpc("getblockhash", rpc("getblockcount"))
  end

  def top_block
    rpc("getblock", top_hash)
  end

  def connection_count
    rpc("getconnectioncount").to_i
  end

  def info
    rpc "getinfo"
  end

  def new_address(account = "")
    rpc "getnewaddress", account
  end
end


