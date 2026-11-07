using FrameWork.Logger;
using FrameWork.NetWork;
using WorldServer.Lobby;

namespace WorldServer.TCP.Packets
{
    [PacketHandlerAttribute(PacketHandlerType.TCP, (int)Opcodes.ASK_WORLD_ENTER, "onAskWorldEnter")]
    public class WORLD_ENTER : IPacketHandler
    {
        public int HandlePacket(BaseClient client, PacketIn packet)
        {
            WorldClient cclient = (WorldClient)client;
            uint accountId = packet.GetUint32Reversed();
            Log.Debug("AskWorldEnter", "New client! Account = " + accountId);

            // The lobby announces every account (with the chosen character and session key)
            // before the client connects here. Take that entry out so it can only be used once.
            Acc expected;
            lock (Program.expectingAccounts)
            {
                if (Program.expectingAccounts.TryGetValue(accountId, out expected))
                    Program.expectingAccounts.Remove(accountId);
            }

            if (expected == null)
            {
                // Without the session key from the lobby there is no way to encrypt a reply,
                // so the only safe answer is to drop the connection.
                Log.Error("AskWorldEnter", "Account " + accountId + " was not announced by the lobby, disconnecting");
                cclient.Disconnect();
                return 0;
            }

            cclient.account = expected;
            cclient.Account = Databases.AccountTable.SingleOrDefault(a => a.Index == accountId);
            // Look the character up by the id the lobby sent: an account can own several characters.
            cclient.Character = Databases.CharacterTable.SingleOrDefault(c => c.Index == expected.Character && c.AccountIndex == accountId);
            if (cclient.Account.Index < 1 || cclient.Character.Index < 1)
            {
                Log.Error("AskWorldEnter", "Account " + accountId + " or character " + expected.Character + " not found in database, disconnecting");
                cclient.Disconnect();
                return 0;
            }

            PacketOut Out = new PacketOut((uint)Opcodes.ANS_WORLD_ENTER);
            Out.WriteInt32Reverse((int)ResponseCodes.RC_SUCCESS);
            Out.WriteUInt32Reverse(expected.Character);
            Out.WriteInt64Reverse(TCPManager.GetTimeStamp());
            Out.WriteFloat(5.00f);
            Out.WriteByte(cclient.Character.IsGroupPublic);
            Out.WriteByte(cclient.Character.GroupInvite);
            Out.WriteInt32Reverse(0);
            Out.WriteInt32Reverse(0);
            Out.WriteInt32Reverse(0);
            Out.WriteInt32Reverse(0);
            Out.WriteInt32Reverse(0);
            Out.WriteByte(1); //has completed tutorial
            Out.WriteByte(cclient.Character.LFG);
            Out.WriteByte(0);

            cclient.Crypto = new Encryption(expected.SessionId);
            cclient.Send(new DISTRICT_LIST());
            cclient.Send(Out);
            return 0;
        }
    }
}
