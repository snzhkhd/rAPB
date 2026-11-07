using FrameWork.Logger;
using FrameWork.NetWork;
using System;
using System.Net;

namespace WorldServer.TCP.Packets
{
    [PacketHandlerAttribute(PacketHandlerType.TCP, (int)Opcodes.ASK_DISTRICT_ENTER, "onAskDistrictEnter")]
    public class DISTRICT_ENTER : IPacketHandler
    {
        // World -> district message announcing a player:
        //   byte    '1'
        //   uint32  account id (little-endian)
        //   byte[16] XXTEA key for this player's UDP traffic
        //   byte[20] handshake hash the client puts in its first UDP packet
        public const int DistrictAnnounceSize = 1 + 4 + 16 + 20;

        public int HandlePacket(BaseClient client, PacketIn packet)
        {
            Send((WorldClient)client);
            return 0;
        }

        public static void Send(WorldClient cclient)
        {
            PacketOut Out = new PacketOut((uint)Opcodes.ANS_DISTRICT_ENTER);
            IPAddress address;
            if (cclient.Reserved == null || cclient.Reserved.tcp == null || !IPAddress.TryParse(cclient.Reserved.IP, out address)
                || address.AddressFamily != System.Net.Sockets.AddressFamily.InterNetwork)
            {
                Log.Error("DistrictEnter", "No usable district reserved for this client");
                Out.WriteUInt32Reverse((uint)ResponseCodes.RC_DISTRICT_RESERVE_DISTRICT_OFFLINE);
                cclient.Send(Out);
                return;
            }

            ulong timestamp = (ulong)TCPManager.GetTimeStamp();

            // handshakeHash = SHA1(sessionKey || timestamp), encryptionKey = SHA1(sessionKey || handshakeHash)[0..16]
            // TODO: confirm the byte order the client uses for the timestamp when it builds the same hash.
            byte[] timestampBytes = BitConverter.GetBytes(timestamp);
            Sha1Digest sha1 = new Sha1Digest();
            sha1.BlockUpdate(cclient.Crypto.Key, 0, cclient.Crypto.Key.Length);
            sha1.BlockUpdate(timestampBytes, 0, timestampBytes.Length);
            byte[] handshakeHash = new byte[sha1.GetDigestSize()];
            sha1.DoFinal(handshakeHash, 0);

            sha1 = new Sha1Digest();
            sha1.BlockUpdate(cclient.Crypto.Key, 0, cclient.Crypto.Key.Length);
            sha1.BlockUpdate(handshakeHash, 0, handshakeHash.Length);
            byte[] encryptionHash = new byte[sha1.GetDigestSize()];
            sha1.DoFinal(encryptionHash, 0);

            // Tell the district server first, so it already knows the player when the first UDP packet arrives.
            byte[] announce = new byte[DistrictAnnounceSize];
            announce[0] = 0x31;
            Buffer.BlockCopy(BitConverter.GetBytes((uint)cclient.Account.Index), 0, announce, 1, 4);
            Buffer.BlockCopy(encryptionHash, 0, announce, 5, 16);
            Buffer.BlockCopy(handshakeHash, 0, announce, 21, 20);
            try
            {
                cclient.Reserved.tcp.Client.Send(announce);
            }
            catch (Exception e)
            {
                Log.Error("DistrictEnter", "Could not reach " + cclient.Reserved + ": " + e.Message);
                Out.WriteUInt32Reverse((uint)ResponseCodes.RC_DISTRICT_RESERVE_DISTRICT_OFFLINE);
                cclient.Send(Out);
                return;
            }

            Out.WriteInt32((int)ResponseCodes.RC_SUCCESS);
            Out.Write(address.GetAddressBytes(), 0, 4);
            Out.WriteUInt16Reverse(cclient.Reserved.Port);
            Out.WriteUInt64Reverse(timestamp);
            cclient.Send(Out);
            Log.Info("DistrictEnter", "Account " + cclient.Account.Index + " sent to " + cclient.Reserved + " at " + cclient.Reserved.IP + ":" + cclient.Reserved.Port);
        }
    }
}
