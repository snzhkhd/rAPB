using FrameWork.Logger;
using FrameWork.NetWork;
using System.Collections.Generic;

namespace WorldServer.TCP.Packets
{
    [PacketHandlerAttribute(PacketHandlerType.TCP, (int)Opcodes.ASK_DISTRICT_RESERVE, "onAskDistrictReserve")]
    public class DISTRICT_RESERVE : IPacketHandler
    {
        public int HandlePacket(BaseClient client, PacketIn packet)
        {
            WorldClient cclient = (WorldClient)client;
            // Districts are keyed as (type << 24) + id, the same value the client sends here.
            uint code = packet.GetUint32();
            byte requestedType = (byte)(code >> 24);
            cclient.Reserved = null;

            Districts.District found = null;
            bool sawFull = false;
            lock (Program.districtsListener.Districts)
            {
                // Prefer the exact district the client picked...
                Districts.District exact;
                if (Program.districtsListener.Districts.TryGetValue(code, out exact))
                {
                    if (exact.isFull() == 0) found = exact;
                    else sawFull = true;
                }

                // ...otherwise any district of the same type that still has room.
                if (found == null)
                {
                    foreach (KeyValuePair<uint, Districts.District> district in Program.districtsListener.Districts)
                    {
                        if (district.Value.Type != requestedType) continue;
                        if (district.Value.isFull() != 0)
                        {
                            sawFull = true;
                            continue;
                        }
                        found = district.Value;
                        break;
                    }
                }
            }

            PacketOut Out = new PacketOut((uint)Opcodes.ANS_DISTRICT_RESERVE);
            if (found != null)
            {
                cclient.Reserved = found;
                Log.Info("DistrictReserve", "Account " + cclient.Account.Index + " reserved " + found + " (requested 0x" + code.ToString("X8") + ")");
                Out.WriteUInt32Reverse((uint)ResponseCodes.RC_SUCCESS);
                Out.WriteUInt32(found.Key);
                Out.WriteByte(cclient.Character.GroupStatus);
                Out.WriteByte(0);
            }
            else
            {
                ResponseCodes rc = sawFull ? ResponseCodes.RC_DISTRICT_RESERVE_FULL : ResponseCodes.RC_DISTRICT_RESERVE_INVALID_DISTRICT;
                Log.Error("DistrictReserve", "Account " + cclient.Account.Index + " could not reserve 0x" + code.ToString("X8") + ": " + rc);
                Out.WriteUInt32Reverse((uint)rc);
                Out.WriteUInt32(0);
                Out.WriteByte(0);
                Out.WriteByte(0);
            }
            cclient.Send(Out);

            // The reservation answer goes out first, then the district address and keys.
            if (found != null) DISTRICT_ENTER.Send(cclient);
            return 0;
        }
    }
}
