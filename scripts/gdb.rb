class UniversalBrewedPython < Requirement
  satisfy { archs_for_command("python").universal? }

  def message; <<-EOS.undent
    A build of GDB using a brewed Python was requested, but Python is not
    a universal build.

    GDB requires Python to be built as a universal binary or it will fail
    if attempting to debug a 32-bit binary on a 64-bit host.
    EOS
  end
end

class Gdb < Formula
  desc "GNU debugger"
  homepage "https://www.gnu.org/software/gdb/"
  url "https://ftpmirror.gnu.org/gdb/gdb-7.11.1.tar.xz"
  mirror "https://ftp.gnu.org/gnu/gdb/gdb-7.11.1.tar.xz"
  sha256 "e9216da4e3755e9f414c1aa0026b626251dfc57ffe572a266e98da4f6988fc70"

  patch :p0, :DATA

  bottle do
    sha256 "ce9602dbc92b1e1d986a1a5536fcef9baf3f703d08a86c4d18e4b411b20b2a4c" => :sierra
    sha256 "90b608379fefd418b72e6b73ae1bde9014d94b9f366259cbc3fea99dc63985b1" => :el_capitan
    sha256 "588dcb9acd832060e189004a4c7fef14b7a3bdeda3a7780b1f1bb8106c810327" => :yosemite
    sha256 "07db094029ff33ec19e0b90633f9a2b8fcceaec14d2bf30f7824b618ce993a3e" => :mavericks
  end

  deprecated_option "with-brewed-python" => "with-python"

  option "with-python", "Use the Homebrew version of Python; by default system Python is used"
  option "with-version-suffix", "Add a version suffix to program"
  option "with-all-targets", "Build with support for all targets"

  depends_on "pkg-config" => :build
  depends_on "python" => :optional
  depends_on "guile" => :optional

  if build.with? "python"
    depends_on UniversalBrewedPython
  end

  def install
    args = [
      "--prefix=#{prefix}",
      "--disable-debug",
      "--disable-dependency-tracking",
      "--enable-targets=all",
    ]

    args << "--with-guile" if build.with? "guile"
    args << "--enable-targets=all" if build.with? "all-targets"

    if build.with? "python"
      args << "--with-python=#{HOMEBREW_PREFIX}"
    else
      args << "--with-python=/usr"
    end

    if build.with? "version-suffix"
      args << "--program-suffix=-#{version.to_s.slice(/^\d/)}"
    end

    system "./configure", *args
    system "make"
    system "make", "install"

    # Remove conflicting items with binutils
    rm_rf include
    rm_rf lib
    rm_rf share/"locale"
    rm_rf share/"info"
  end

  def caveats; <<-EOS.undent
    gdb requires special privileges to access Mach ports.
    You will need to codesign the binary. For instructions, see:

      https://sourceware.org/gdb/wiki/BuildingOnDarwin
    EOS
  end

  test do
    system bin/"gdb", bin/"gdb", "-configuration"
  end
end

__END__


diff a/gdb/remote.c b/gdb/remote.c
--- gdb/remote.c  	2016-04-14 11:13:49.962628700 +0300
+++ gdb/remote.c	2016-04-14 11:15:38.257783400 +0300
@@ -7181,8 +7181,28 @@
   buf_len = strlen (rs->buf);
 
   /* Further sanity checks, with knowledge of the architecture.  */
+// HACKFIX for changing architectures for qemu. It's ugly. Don't use, unless you have to.
+  // Just a tiny modification of the patch of Matias Vara (http://forum.osdev.org/viewtopic.php?f=13&p=177644)
   if (buf_len > 2 * rsa->sizeof_g_packet)
-    error (_("Remote 'g' packet reply is too long: %s"), rs->buf);
+    {
+      warning (_("Assuming long-mode change. [Remote 'g' packet reply is too long: %s]"), rs->buf);
+      rsa->sizeof_g_packet = buf_len ;
+
+      for (i = 0; i < gdbarch_num_regs (gdbarch); i++)
+        {
+          if (rsa->regs[i].pnum == -1)
+            continue;
+
+          if (rsa->regs[i].offset >= rsa->sizeof_g_packet)
+            rsa->regs[i].in_g_packet = 0;
+          else
+            rsa->regs[i].in_g_packet = 1;
+        }
+
+      // HACKFIX: Make sure at least the lower half of EIP is set correctly, so the proper
+      // breakpoint is recognized (and triggered).
+      rsa->regs[8].offset = 16*8;
+    }
 
   /* Save the size of the packet sent to us by the target.  It is used
      as a heuristic when determining the max size of packets that the

