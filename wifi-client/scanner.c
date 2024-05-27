/**
 * @file scanner.c
 * @author your name (you@domain.com)
 * @brief Capture wifi beacon frames.
 * @version 0.1
 * @date 2024-05-27
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <iwlib.h>

int main(void) {
  wireless_scan_head head;
  wireless_scan *result;
  iwrange range;
  int sock;

  /* Open socket to kernel */
  sock = iw_sockets_open();

  /* Get some metadata to use for scanning */
  if (iw_get_range_info(sock, "wlan1", &range) < 0) {
    printf("Error during iw_get_range_info. Aborting.\n");
    exit(2);
  }

  /* Perform the scan */
  int res =iw_scan(sock, "wlan1", range.we_version_compiled, &head);
  if (res < 0) {
    printf("Error during iw_scan. Aborting: %d %d\n", res, errno);
    exit(2);
  }

  /* Traverse the results */
  result = head.result;
  while (NULL != result) {
    printf("%s\n", result->b.essid);
    result = result->next;
  }

  exit(0);
}