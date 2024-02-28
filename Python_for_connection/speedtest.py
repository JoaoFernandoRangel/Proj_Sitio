import speedtest

wifi = speedtest.Speedtest() # Adjust the constructor call accordingly

print("Getting download speed...")
download = wifi.download()

print(download)